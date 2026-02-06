#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
    TokError,
    TokPlus,
    TokInt,
    TokString,
    TokLParen,
    TokRParen,
    TokStar,
} TokenType;

typedef struct {
    TokenType type;
    const char* value;
} Token;

typedef struct {
    Token* data;
    unsigned int size;
    unsigned int capacity;
} VecTokens;

typedef struct {
    const char* buffer;
    unsigned int cursor;
    unsigned int size;
} CharStream;

// Функции для работы с токенами
void add_token(VecTokens* tokens, Token token);
Token get_token(VecTokens* tokens, unsigned idx);

// Функции для работы с потоком
void skipWhitespace(CharStream* stream);
char peek(CharStream* stream);
char get(CharStream* stream);
_Bool eof(CharStream* stream);

// Парсинг токенов
Token parseTokInt(CharStream* stream);
Token parseTokPlus(CharStream* stream);
Token parseTokString(CharStream* stream);
Token parseTokLParen(CharStream* stream);
Token parseTokRParen(CharStream* stream);
Token parseTokStar(CharStream* stream);
VecTokens tokenize(CharStream* stream);

extern Token error_token;

#endif
