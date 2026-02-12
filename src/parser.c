#include "parser.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "tokenizer.h"

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
const char* json_read_string(CharStream* stream){
    skipWhitespace(stream);
    if (cs_peek(stream) != '\"'){
        return 0;
    }
    cs_get(stream);
    char* value = malloc(MAX_STRING_LENGTH + 1);
    unsigned int size = 0;
    while(!cs_eof(stream) && (cs_peek(stream) != '\"')){
        value[size] = cs_get(stream);
        size += 1;
        if(size > MAX_STRING_LENGTH){
            return 0;
        }
    }
    cs_get(stream);
    value[size] = '\0';
    return value;
}

int try_consume_with_ws(CharStream* stream, const char* pattern){
    unsigned int saved_cursor = get_cursor(stream);
    int size = 0;
    while(!cs_eof(stream) && (pattern[size] != '\0')){
        skipWhitespace(stream);
        if(cs_peek(stream) != pattern[size]) {
            break;
        }
        cs_get(stream);
        size += 1;
    }
    if (pattern[size] != '\0'){
        set_cursor(stream, saved_cursor);
        return 0;
    }
    return 1;
}

ASTNode* json_load_block(CharStream* stream){
    skipWhitespace(stream);
    if (cs_peek(stream) != '{'){
        return 0;
    }
    cs_get(stream);

    ASTNode* result = malloc(sizeof(*result));

    if(!try_consume_with_ws(stream, "\"type\":")){
        return 0;
    }
    const char* type_str = json_read_string(stream);
    if(!type_str){
        return 0;
    }

    if (str_eq(type_str, "ADD") || str_eq(type_str, "MUL")){
        if (!try_consume_with_ws(stream, ",\"lhs\":")){
            return 0;
        }
        ASTNode* lhs = json_load_block(stream);
        if (!lhs){
            return 0;
        }
        if (!try_consume_with_ws(stream, ",\"rhs\":")){
            return 0;
        }
        ASTNode* rhs = json_load_block(stream);
        if (!rhs){
            return 0;
        }
        if(str_eq(type_str, "ADD")) result->type = ADD;
        else result->type = MUL;
        result->data.binop.lhs = lhs;
        result->data.binop.rhs = rhs;
    }
    else if (str_eq(type_str, "ID")){
        if (!try_consume_with_ws(stream, ",\"value\":")) {
            return 0;
        }
        const char* val = json_read_string(stream);
        result->type = ID;
        result->data.ident.value = val;
    }
    else if (str_eq(type_str, "NUM")){
        if (!try_consume_with_ws(stream, ",\"value\":")) {
            return 0;
        }
        const char* val = json_read_string(stream);
        result->type = NUM;
        result->data.number.value = val;
    }
    else{
        return 0;
    }

    skipWhitespace(stream);
    if (!cs_eof(stream) && cs_peek(stream) == '}'){
        cs_get(stream);
    }
    return result;
}

ASTNode* ast_load(const char* json){
    CharStream stream = {json, 0, strlen(json)};
    return json_load_block(&stream);
}