#include "lib/memory.h"
#include "tokenizer.h"


Token error_token = {.type = TokError, .value = 0};

void extVecTokens(VecTokens* tokens){
    unsigned int new_capacity = tokens->capacity * 2 + 1;
    Token* new_data = malloc(new_capacity * sizeof(Token));
    for (int i = 0; i < tokens->size; i++){
        new_data[i] = tokens->data[i];
    }
    tokens->capacity = new_capacity;
    tokens->data = new_data;
}

void add_token(VecTokens* tokens, Token token){
    if (tokens->size >= tokens->capacity){
        extVecTokens(tokens);
    }
    tokens->data[tokens->size] = token;
    tokens->size+= 1;
}

Token get_token(VecTokens* tokens, unsigned idx){
    return tokens->data[idx];
}

Token ts_peek(TokStream* stream){
    if (stream->cursor >= stream->size){
        return error_token;
    }
    return stream->data[stream->cursor];
}

Token ts_get(TokStream* stream){
    Token t = stream->data[stream->cursor];
    stream->cursor += 1;
    return t;
}

int ts_eof(TokStream* stream){
    return stream->cursor >= stream->size;
}

TokStream make_tokstream(VecTokens* tokens){
    TokStream s = {.data = tokens->data, .cursor = 0, .size = tokens->size};
    return s;
}

int isdigit(char c){
    return '0' <= c && c <= '9';
}

int isalpha(char c){
    return ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z');
}

int try_parse_keyword(CharStream* stream, const char* keyword){
    unsigned int saved = get_cursor(stream);
    int i = 0;
    while (keyword[i] != '\0'){
        if (cs_eof(stream) || cs_peek(stream) != keyword[i]){
            set_cursor(stream, saved);
            return 0;
        }
        cs_get(stream);
        i += 1;
    }
    if (!cs_eof(stream) && isalpha(cs_peek(stream))){
        set_cursor(stream, saved);
        return 0;
    }
    return 1;
}

Token parseTokInt(CharStream* stream){
    if (!isdigit(cs_peek(stream)))  {
        return error_token;
    }
    char* value = malloc(sizeof(char) * (MAX_INT_LENGTH + 1));
    int size = 0;
    while(isdigit(cs_peek(stream)) && size < 10) {
        value[size] = cs_get(stream);
        size++; 
    }
    value[size] = '\0';
    Token token = {TokInt, value};
    return token;
}

Token parseTokPlus(CharStream* stream){
    if(cs_peek(stream) == '+'){
        cs_get(stream);
        Token token = {TokPlus, 0};
        return token;
    }
    return error_token;
}
Token parseTokMinus(CharStream* stream){
    if(cs_peek(stream) == '-'){
        cs_get(stream);
        Token token = {TokMinus, 0};
        return token;
    }
    return error_token;
    
}

Token parseTokString(CharStream* stream){
    if(cs_peek(stream) != '\"'){
        return error_token;
    }
    cs_get(stream);
    char* value = malloc(sizeof(char) * (1 + MAX_STRING_LENGTH));
    int size = 0;
    while(!cs_eof(stream) && cs_peek(stream) != '\"'){
        value[size] = cs_get(stream); 
        size+= 1;
    }
    if (cs_eof(stream)){
        return error_token;
    }
    cs_get(stream);
    
    Token token = {TokString, value};
    return token;
}

Token parseTokLParen(CharStream* stream){
    if(cs_peek(stream) == '('){
        cs_get(stream);
        Token token = {TokLParen, 0};
        return token;
    }
    return error_token;
}

Token parseTokRParen(CharStream* stream){
    if (cs_peek(stream) == ')'){
        cs_get(stream);
        Token token = {TokRParen, 0};
        return token;
    }
    return error_token;
}

Token parseTokStar(CharStream* stream){
    if (cs_peek(stream) == '*'){
        cs_get(stream);
        Token token = {TokStar, 0};
        return token;
    }
    return error_token;
}

Token parseTokSlash(CharStream* stream){
    if (cs_peek(stream) == '/'){
        cs_get(stream);
        Token token = {TokSlash, 0};
        return token;
    }
    return error_token;
}

Token parseTokExpr(CharStream* stream){
    if (cs_peek(stream) == '^'){
        cs_get(stream);
        Token token = {TokExp, 0};
        return token;
    }
    return error_token;
}

Token parseTokAssign(CharStream* stream){
    if (cs_peek(stream) == '='){
        cs_get(stream);
        Token token = {TokAssign, 0};
        return token;
    }
    return error_token;
}

Token parseTokEqual(CharStream* stream){
    unsigned int saved = get_cursor(stream);
    if (cs_peek(stream) == '='){
        cs_get(stream);
        if (!cs_eof(stream) && cs_peek(stream) == '='){
            cs_get(stream);
            Token token = {TokEqual, 0};
            return token;
        }
    }
    set_cursor(stream, saved);
    return error_token;
}

Token parseTokNotequal(CharStream* stream){
    unsigned int saved = get_cursor(stream);
    if (cs_peek(stream) == '~'){
        cs_get(stream);
        if (!cs_eof(stream) && cs_peek(stream) == '='){
            cs_get(stream);
            Token token = {TokNotequal, 0};
            return token;
        }
    }
    set_cursor(stream, saved);
    return error_token;
}

Token parseTokLEq(CharStream* stream){
    unsigned int saved = get_cursor(stream);
    if (cs_peek(stream) == '<'){
        cs_get(stream);
        if (!cs_eof(stream) && cs_peek(stream) == '='){
            cs_get(stream);
            Token token = {TokLEq, 0};
            return token;
        }
    }
    set_cursor(stream, saved);
    return error_token;
}

Token parseTokGEq(CharStream* stream){
    unsigned int saved = get_cursor(stream);
    if (cs_peek(stream) == '>'){
        cs_get(stream);
        if (!cs_eof(stream) && cs_peek(stream) == '='){
            cs_get(stream);
            Token token = {TokGEq, 0};
            return token;
        }
    }
    set_cursor(stream, saved);
    return error_token;
}

Token parseTokLT(CharStream* stream){
    if (cs_peek(stream) == '<'){
        cs_get(stream);
        Token token = {TokLT, 0};
        return token;
    }
    return error_token;
}

Token parseTokGT(CharStream* stream){
    if (cs_peek(stream) == '>'){
        cs_get(stream);
        Token token = {TokGT, 0};
        return token;
    }
    return error_token;
}
Token parseTokLBracket(CharStream* stream){
    if (cs_peek(stream) == '['){
        cs_get(stream);
        Token token = {TokLBracket, 0};
        return token;
    }
    return error_token;
}

Token parseTokRBracket(CharStream* stream){
    if (cs_peek(stream) == ']'){
        cs_get(stream);
        Token token = {TokRBracket, 0};
        return token;
    }
    return error_token;
}

Token parseTokLBrace(CharStream* stream){
    if (cs_peek(stream) == '{'){
        cs_get(stream);
        Token token = {TokLBrace, 0};
        return token;
    }
    return error_token;
}

Token parseTokRBrace(CharStream* stream){
    if (cs_peek(stream) == '}'){
        cs_get(stream);
        Token token = {TokRBrace, 0};
        return token;
    }
    return error_token;
}

Token parseTokColon(CharStream* stream){
    if (cs_peek(stream) == ':'){
        cs_get(stream);
        Token token = {TokColon, 0};
        return token;
    }
    return error_token;
}

Token parseTokDot(CharStream* stream){
    if (cs_peek(stream) == '.'){
        cs_get(stream);
        Token token = {TokDot, 0};
        return token;
    }
    return error_token;
}
Token parseTokComma(CharStream* stream){
    if (cs_peek(stream) == ','){
        cs_get(stream);
        Token token = {TokComma, 0};
        return token;
    }
    return error_token;
}

Token parseTokDDot(CharStream* stream){
    unsigned int saved = get_cursor(stream);
    if (cs_peek(stream) == '.'){
        cs_get(stream);
        if (!cs_eof(stream) && cs_peek(stream) == '.'){
            cs_get(stream);
            Token token = {TokDDot, 0};
            return token;
        }
    }
    set_cursor(stream, saved);
    return error_token;
}

Token parseTokNot(CharStream* stream){
    if (try_parse_keyword(stream, "not")){
        Token token = {TokNot, 0};
        return token;
    }
    return error_token;
}

Token parseTokAnd(CharStream* stream){
    if (try_parse_keyword(stream, "and")){
        Token token = {TokAnd, 0};
        return token;
    }
    return error_token;
}

Token parseTokOr(CharStream* stream){
    if (try_parse_keyword(stream, "or")){
        Token token = {TokOr, 0};
        return token;
    }
    return error_token;
}

Token parseTokIdent(CharStream* stream){
    if (isalpha(cs_peek(stream))){
        char* name = malloc(sizeof(*name) * MAX_IDENTIFIER_LENGTH);
        unsigned int size = 0; 
        while (!cs_eof(stream) && isalpha(cs_peek(stream))) {
            name[size] = cs_get(stream);
            size+= 1; 
        }
        name[size] = '\0';
        Token token = {TokIdent, name};
        return token;
    } 
   return error_token;
}

Token parseTokIf(CharStream* stream){
    if (try_parse_keyword(stream, "if")){
        Token token = {TokIf, 0};
        return token;
    }
    return error_token;
}

Token parseTokThen(CharStream* stream){
    if (try_parse_keyword(stream, "then")){
        Token token = {TokThen, 0};
        return token;
    }
    return error_token;
}

Token parseTokElse(CharStream* stream){
    if (try_parse_keyword(stream, "else")){
        Token token = {TokElse, 0};
        return token;
    }
    return error_token;
}

Token parseTokEnd(CharStream* stream){
    if (try_parse_keyword(stream, "end")){
        Token token = {TokEnd, 0};
        return token;
    }
    return error_token;
}

Token parseTokWhile(CharStream* stream){
    if (try_parse_keyword(stream, "while")){
        Token token = {TokWhile, 0};
        return token;
    }
    return error_token;
}

Token parseTokFor(CharStream* stream){
    if (try_parse_keyword(stream, "for")){
        Token token = {TokFor, 0};
        return token;
    }
    return error_token;
}

Token parseTokDo(CharStream* stream){
    if (try_parse_keyword(stream, "do")){
        Token token = {TokDo, 0};
        return token;
    }
    return error_token;
}

Token parseTokIn(CharStream* stream){
    if (try_parse_keyword(stream, "in")){
        Token token = {TokIn, 0};
        return token;
    }
    return error_token;
}

Token parseTokFunction(CharStream* stream){
    if (try_parse_keyword(stream, "function")){
        Token token = {TokFunction, 0};
        return token;
    }
    return error_token;
}

Token parseTokReturn(CharStream* stream){
    if (try_parse_keyword(stream, "return")){
        Token token = {TokReturn, 0};
        return token;
    }
    return error_token;
}

Token parseTokNil(CharStream* stream){
    if (try_parse_keyword(stream, "nil")){
        Token token = {TokNil, 0};
        return token;
    }
    return error_token;
}

VecTokens tokenize(CharStream* stream){
    VecTokens tokens = {.data = 0, .size = 0, .capacity = 0};
    while(!cs_eof(stream)) {
        skipWhitespace(stream);
        if (cs_eof(stream)) break;
        Token token;
        if((token = parseTokInt(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokEqual(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokAssign(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokNotequal(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokLEq(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokGEq(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokDDot(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokNot(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokIf(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokThen(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokElse(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokEnd(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokWhile(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokFor(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokDo(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokIn(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokFunction(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokReturn(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokNil(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokAnd(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokOr(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokPlus(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokMinus(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokStar(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokSlash(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokExpr(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokLT(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokGT(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokLParen(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokRParen(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokLBracket(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokRBracket(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokLBrace(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokRBrace(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokComma(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokColon(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokDot(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokString(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokIdent(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else {
            break;
        }
    }
    return tokens;
}