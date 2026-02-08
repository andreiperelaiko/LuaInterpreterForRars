#include "tokenizer.h"

typedef enum {
    ADD,
    MUL,
    NUM,
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct{
    ASTNode* lhs;
    ASTNode* rhs; 
} BinOp;

typedef struct{
    const char* value; 
} Number;

struct ASTNode {
    ASTNodeType type; 
    union {
        Number number;
        BinOp binop;
    } data;
};

ASTNodeType convert(TokenType type);
int is_operator(TokenType type);

int get_precedence(TokenType type);

ASTNode* parse_expr(TokStream* stream, int precedence);
ASTNode* parse_nud(TokStream* stream);