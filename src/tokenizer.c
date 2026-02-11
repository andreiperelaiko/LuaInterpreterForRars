#include "lib/memory.h"
#include "tokenizer.h"


#define MAX_INT_LENGTH 10
#define MAX_STRING_LENGTH 100
#define MAX_IDENTIFIER_LENGTH 100

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
        cs_peek(stream) == ' ' ||
        cs_peek(stream) == '\n'
    ){
        cs_get(stream);
    } 
}

unsigned int get_cursor(CharStream* stream){
    return stream->cursor;
}

void set_cursor(CharStream* stream, unsigned int cursor){
    stream->cursor = cursor; 
}

char cs_peek(CharStream* stream){
    return stream->buffer[stream->cursor];
}

char cs_get(CharStream* stream){
    char c = stream->buffer[stream->cursor];
    stream->cursor += 1;
    return c;
}

int cs_eof(CharStream* stream) {
    return stream->cursor >= stream->size;  
}

Token ts_peek(TokStream* stream){
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

Token parseTokIdent(CharStream* stream){
    if (isalpha(cs_peek(stream))){
        char* name = malloc(sizeof(*name) * MAX_IDENTIFIER_LENGTH);
        Token token = {TokIdent, name};
        unsigned int size = 0; 
        while (isalpha(cs_peek(stream))) {
            name[size] = cs_get(stream);
            size+= 1; 
        }
        return token;
    } 
   return error_token;
}

VecTokens tokenize(CharStream* stream){
    VecTokens tokens = {.data = 0, .size = 0, .capacity = 0};
    while(!cs_eof(stream)) {
        skipWhitespace(stream);
        Token token;
        if((token = parseTokInt(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokPlus(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokStar(stream)).type != TokError) {
            add_token(&tokens, token);
        }
        else if((token = parseTokIdent(stream)).type != TokError) {
            add_token(&tokens, token);
        }
    }
    return tokens;
}