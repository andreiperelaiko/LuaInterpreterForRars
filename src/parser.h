#include "tokenizer.h"

typedef enum {
    ADD,
    MUL,
    NUM,
    ID,
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct{
    ASTNode* lhs;
    ASTNode* rhs; 
} BinOp;

typedef struct{
    const char* value; 
} Number;

typedef struct{
    const char* value;
} Ident;

struct ASTNode {
    ASTNodeType type; 
    union {
        Number number;
        Ident ident;
        BinOp binop;
    } data;
};

const char* ast_dump(const ASTNode* root);
ASTNode* ast_load(const char* json);

ASTNodeType convert(TokenType type);
int is_operator(TokenType type);

int get_precedence(TokenType type);

ASTNode* parse_expr(TokStream* stream, int precedence);
ASTNode* parse_nud(TokStream* stream);