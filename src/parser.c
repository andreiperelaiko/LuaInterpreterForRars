#include "parser.h"
#include "lib/memory.h"
#include "lib/string.h"

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
        case TokIdent: {
            ts_get(stream);
            ASTNode* node = malloc(sizeof(*node));
            node->type = ID;
            node->data.ident.value = token.value;
        }
        default:
            return 0;
    }
}

const char* ast_dump(const ASTNode* root){
    if (root == 0) return "null";
    switch (root->type){
        case NUM: {
            char* s = str_concat("{\"type\":\"NUM\",\"value\":\"", root->data.number.value);
            return str_concat(s, "\"}");
        }
        case ID: {
            char* s = str_concat("{\"type\":\"ID\",\"value\":\"", root->data.ident.value);
            return str_concat(s, "\"}");
        }
        case ADD:
        case MUL: {
            const char* type = root->type == ADD ? "ADD" : "MUL";
            const char* lhs = ast_dump(root->data.binop.lhs);
            const char* rhs = ast_dump(root->data.binop.rhs);
            char* s = str_concat("{\"type\":\"", type);
            s = str_concat(s, "\",\"lhs\":");
            s = str_concat(s, lhs);
            s = str_concat(s, ",\"rhs\":");
            s = str_concat(s, rhs);
            return str_concat(s, "}");
        }
        default:
            return "null";
    }
}