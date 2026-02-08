#include "parser.h"
#include "lib/memory.h"

int is_operator(TokenType type){
    switch (type){
        case TokPlus:
        case TokStar:
            return 1;
        default:
            return 0; 
    }
}

int get_precedence(TokenType type){
    switch (type){
        case TokPlus:
            return 1;
        case TokStar:
            return 2;
        default:
            return -1;
    }
}

ASTNodeType convert_binop(TokenType type){
    switch (type){
        case TokPlus:
            return ADD;
        case TokStar:
            return MUL;
        default:
            return 0;
    }
}


ASTNode* parse_expr(TokStream* stream, int min_precedence){
    ASTNode* left = parse_nud(stream);
    while (!ts_eof(stream)){
        Token token = ts_peek(stream);
        if (!is_operator(token.type)){
            break;
        }
        int precedence = get_precedence(token.type);
        if (precedence < min_precedence) {
            break;
        }
        ts_get(stream);
        ASTNode* right = parse_expr(stream, precedence + 1);
        ASTNode* node = malloc(sizeof(ASTNode));
        node->type = convert_binop(token.type);
        node->data.binop.lhs = left;
        node->data.binop.rhs = right;
        left = node;
    }
    return left;
}

ASTNode* parse_nud(TokStream* stream){
    Token token = ts_peek(stream);
    switch (token.type){
        case TokInt: {
            ts_get(stream);
            ASTNode* node = malloc(sizeof(ASTNode));
            node->type = NUM;
            node->data.number.value = token.value;
            return node;
        }
        default:
            return 0;
    }
}