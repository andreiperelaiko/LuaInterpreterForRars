#include "lib/memory.h"
#include "tokenizer.h"


#define MAX_INT_LENGTH 10
#define MAX_STRING_LENGTH 100

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

void skipWhitespace(CharStream* stream){
    while(
        peek(stream) == ' ' ||
        peek(stream) == '\n'
    ){
        get(stream);
    } 
}

unsigned int get_cursor(CharStream* stream){
    return stream->cursor;
}

void set_cursor(CharStream* stream, unsigned int cursor){
    stream->cursor = cursor; 
}

char peek(CharStream* stream){
    return stream->buffer[stream->cursor];
}

char get(CharStream* stream){
    char c = stream->buffer[stream->cursor];
    stream->cursor += 1;
    return c;
}

_Bool eof(CharStream* stream) {
    return stream->cursor >= stream->size;  
}

_Bool isdigit(char c){
    return '0' <= c && c <= '9';
}

Token parseTokInt(CharStream* stream){
    if (!isdigit(peek(stream)))  {
        return error_token;
    }
    char* value = malloc(sizeof(char) * (MAX_INT_LENGTH + 1));
    int size = 0;
    while(isdigit(peek(stream)) && size < 10) {
        value[size] = get(stream);
        size++; 
    }
    value[size] = '\0';
    Token token = {TokInt, value};
    return token;
}

Token parseTokPlus(CharStream* stream){
    if(peek(stream) == '+'){
        get(stream);
        Token token = {TokPlus, 0};
        return token;
    }
    return error_token;
}

Token parseTokString(CharStream* stream){
    if(peek(stream) != '\"'){
        return error_token;
    }
    get(stream);
    char* value = malloc(sizeof(char) * (1 + MAX_STRING_LENGTH));
    int size = 0;
    while(!eof(stream) && peek(stream) != '\"'){
        value[size] = get(stream); 
        size+= 1;
    }
    if (eof(stream)){
        return error_token;
    }
    get(stream);
    
    Token token = {TokString, value};
    return token;
}

Token parseTokLParen(CharStream* stream){
    if(peek(stream) == '('){
        get(stream);
        Token token = {TokLParen, 0};
        return token;
    }
    return error_token;
}

Token parseTokRParen(CharStream* stream){
    if (peek(stream) == '('){
        get(stream);
        Token token = {TokRParen, 0};
        return token;
    }
    return error_token;
}

Token parseTokStar(CharStream* stream){
    if (peek(stream) == '*'){
        get(stream);
        Token token = {TokStar, 0};
        return token;
    }
    return error_token;
}

VecTokens tokenize(CharStream* stream){
    VecTokens tokens = {.data = 0, .size = 0, .capacity = 0};
    while(!eof(stream)) {
        skipWhitespace(stream);
        Token token;
        if((token = parseTokInt(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokPlus(stream)).type != TokError) {
            add_token(&tokens, token);
        }
    }
    return tokens;
}