#ifndef TOKENIZER_H
#define TOKENIZER_H

#define MAX_INT_LENGTH 10
#define MAX_STRING_LENGTH 100
#define MAX_IDENTIFIER_LENGTH 100

typedef enum {
    TokError,
    TokPlus,
    TokInt,
    TokString,
    TokLParen,
    TokRParen,
    TokStar,
    TokIdent,
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

typedef struct {
    Token* data;
    unsigned int cursor;
    unsigned int size;
} TokStream;

void add_token(VecTokens* tokens, Token token);
Token get_token(VecTokens* tokens, unsigned idx);

void skipWhitespace(CharStream* stream);
char cs_peek(CharStream* stream);
char cs_get(CharStream* stream);
int cs_eof(CharStream* stream);
unsigned int get_cursor(CharStream* stream);
void set_cursor(CharStream* stream, unsigned int cursor);

Token ts_peek(TokStream* stream);
Token ts_get(TokStream* stream);
int ts_eof(TokStream* stream);
TokStream make_tokstream(VecTokens* tokens);

Token parseTokInt(CharStream* stream);
Token parseTokPlus(CharStream* stream);
Token parseTokString(CharStream* stream);
Token parseTokLParen(CharStream* stream);
Token parseTokRParen(CharStream* stream);
Token parseTokStar(CharStream* stream);
Token parseTokIdent(CharStream* stream);
VecTokens tokenize(CharStream* stream);

extern Token error_token;

#endif
