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
    NIL,
    NUM,
    ID,
    STRING,
    TABLE,
    IDX,
    CALL,
    NOT,
    NEG,
    IF_STMT,
    BLOCK,
    WHILE_STMT,
    FOR_STMT,
    FOR_NUM_STMT,
    CALL_STMT,
    ASSIGN_STMT,
    FUNC_STMT,
    RETURN_STMT,
} ASTNodeType;

#define AST_NODE_TYPE_COUNT (RETURN_STMT + 1)

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

#define MAX_TABLE_ITEMS 64
typedef struct{
    ASTNode** items;
    unsigned int items_cnt;
} TableCons;

#define MAX_BLOCK_STMTS 100
typedef struct{
    ASTNode** stmts;
    unsigned int stmts_cnt;
} Block;

typedef struct{
    ASTNode* cond;
    ASTNode* then_block;
    ASTNode* else_block;
} IfNode;

typedef struct{
    ASTNode* cond;
    ASTNode* body;
} WhileNode;

typedef struct{
    const char* name;
    ASTNode* iterable;
    ASTNode* body;
} ForNode;

typedef struct{
    const char* name;
    ASTNode* start;
    ASTNode* stop;
    ASTNode* step;
    ASTNode* body;
} ForNumNode;

typedef struct{
    ASTNode* call;
} CallStmt;

typedef struct{
    ASTNode* lhs;
    ASTNode* rhs;
} AssignStmt;

#define MAX_FUNC_PARAMS 16
typedef struct{
    const char* name;
    const char** params;
    unsigned int params_cnt;
    ASTNode* body;
} FuncStmt;

typedef struct{
    ASTNode* value;
} ReturnStmt;

struct ASTNode {
    ASTNodeType type; 
    union {
        Number number;
        Ident ident;
        String string;
        TableCons table_cons;
        BinOp binop;
        UnOp unop;
        Index index;
        Call call;
        Block block;
        IfNode if_node;
        WhileNode while_node;
        ForNode for_node;
        ForNumNode for_num_node;
        CallStmt call_stmt;
        AssignStmt assign_stmt;
        FuncStmt func_stmt;
        ReturnStmt return_stmt;
    } data;
};


const char* ast_dump(const ASTNode* root);
ASTNode* ast_load(const char* json);

#endif