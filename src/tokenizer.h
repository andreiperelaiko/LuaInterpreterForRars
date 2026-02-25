#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "lib/char_stream.h"

#define MAX_INT_LENGTH 10
#define MAX_STRING_LENGTH 100
#define MAX_IDENTIFIER_LENGTH 100

typedef enum {
    TokError,     
    TokInt,
    TokAssign,    // =
    TokPlus,      // + 
    TokMinus,     // -
    TokSlash,     // /
    TokExp,       // ^
    TokEqual,     // ==
    TokNotequal,  // ~=
    TokLEq,       // <=
    TokGEq,       // >=
    TokLT,        // <
    TokGT,        // >
    TokAnd,       // and
    TokOr,        // or
    TokComma,     // ,
    TokDot,       // .
    TokDDot,      // ..
    TokColon,     // :
    TokLBracket,  // [
    TokRBracket,  // ]
    TokNot,       // not
    TokLParen,    // (
    TokRParen,    // )
    TokStar,      // *
    TokIf,        // if
    TokThen,      // then
    TokElse,      // else
    TokEnd,       // end
    TokWhile,     // while
    TokFor,       // for
    TokDo,        // do
    TokIn,        // in
    TokFunction,  // function
    TokNil,       // nil
    TokIdent,
    TokString,
} TokenType;

#define TOKEN_TYPE_COUNT (TokString + 1)

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
    Token* data;
    unsigned int cursor;
    unsigned int size;
} TokStream;

void add_token(VecTokens* tokens, Token token);
Token get_token(VecTokens* tokens, unsigned idx);

Token ts_peek(TokStream* stream);
Token ts_get(TokStream* stream);
int ts_eof(TokStream* stream);
TokStream make_tokstream(VecTokens* tokens);

Token parseTokInt(CharStream* stream);
Token parseTokMinus(CharStream* stream);
Token parseTokSlash(CharStream* stream);
Token parseTokExpr(CharStream* stream);
Token parseTokAssign(CharStream* stream);
Token parseTokEqual(CharStream* stream);
Token parseTokNotequal(CharStream* stream);
Token parseTokLEq(CharStream* stream);
Token parseTokGEq(CharStream* stream);
Token parseTokLT(CharStream* stream);
Token parseTokGT(CharStream* stream);
Token parseTokAnd(CharStream* stream);
Token parseTokOr(CharStream* stream);
Token parseTokDot(CharStream* stream);
Token parseTokComma(CharStream* stream);
Token parseTokDDot(CharStream* stream);
Token parseTokLBracket(CharStream* stream);
Token parseTokRBracket(CharStream* stream);
Token parseTokColon(CharStream* stream);
Token parseTokPlus(CharStream* stream);
Token parseTokString(CharStream* stream);
Token parseTokLParen(CharStream* stream);
Token parseTokRParen(CharStream* stream);
Token parseTokNot(CharStream* stream);
Token parseTokIf(CharStream* stream);
Token parseTokThen(CharStream* stream);
Token parseTokElse(CharStream* stream);
Token parseTokEnd(CharStream* stream);
Token parseTokWhile(CharStream* stream);
Token parseTokFor(CharStream* stream);
Token parseTokDo(CharStream* stream);
Token parseTokIn(CharStream* stream);
Token parseTokFunction(CharStream* stream);
Token parseTokNil(CharStream* stream);
Token parseTokStar(CharStream* stream);
Token parseTokIdent(CharStream* stream);
VecTokens tokenize(CharStream* stream);

extern Token error_token;

#endif
