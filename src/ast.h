#ifndef AST_H
#define AST_H

#define MAX_CALL_ARGS 10
typedef enum {
    ADD,
    MUL,
    DIV,
    SUB,
    EXP,
    EQ,
    NEQ,
    LEQ,
    GEQ,
    LT,
    GT,
    AND,
    OR,
    NUM,
    ID,
    STRING,
    IDX,
    CALL,
    NOT,
    NEG,
} ASTNodeType;

typedef struct ASTNode ASTNode;

typedef struct{
    ASTNode* lhs;
    ASTNode* rhs; 
} BinOp;

typedef struct{
    const ASTNode* value;
} UnOp;

typedef struct{
    const char* value; 
} Number;

typedef struct{
    const char* value;
} Ident;

typedef struct{
    const char* value;
} String;

typedef struct{
    ASTNode* container;
    ASTNode* index;
} Index;

typedef struct{
    ASTNode* calle;
    ASTNode** args;
    unsigned int args_cnt;
} Call;


struct ASTNode {
    ASTNodeType type; 
    union {
        Number number;
        Ident ident;
        String string;
        BinOp binop;
        UnOp unop;
        Index index;
        Call call;
    } data;
};


const char* ast_dump(const ASTNode* root);
ASTNode* ast_load(const char* json);

#endif