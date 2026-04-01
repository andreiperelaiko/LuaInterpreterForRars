#include "ast.h"

#ifndef INTERPRETER_H
#define INTERPRETER_H

typedef enum {
    VAL_NIL,
    VAL_NUMBER,
    VAL_BOOL,
    VAL_STRING,
    VAL_TABLE,
    VAL_FUNCTION,
} ValueType;

typedef enum {
    C_FUNCTION,
    LUA_FUNCTION
} FunctionType;


typedef struct Table Table;
typedef struct Value Value;
typedef struct Pair Pair;
typedef struct Environment Environment;
typedef struct LValue LValue;
typedef Value (*c_func)(Environment* env, Value* args, unsigned int args_cnt);

typedef struct{
    FunctionType type;
    Value* args;
    unsigned int args_cnt;
    Environment* env;
    union {
        ASTNode* lua;
        c_func c;
    } body;
} Function;



struct Value{
    ValueType type;
    union {
        int number;
        int boolean;
        const char* string;
        Table* table;
        Function* func;
    } data;
};

struct Pair{
    Value first;
    Value second;
};

struct Table {
    Pair* pairs;
    unsigned int capacity;
    unsigned int size;
};

struct Environment {
    Table* table;
    Environment* parent;
    int has_return;
    Value return_value;
};

struct LValue{
    Table* table;
    Value key;
};


Table* create_table();
Environment* create_env();
void table_set(Table* table, Value key, Value value);
Value table_get(Table* table, Value key);
void env_set(Environment* env, Value key, Value value);
Value env_get(Environment* env, Value key);


extern Table _G;
Value eval(ASTNode* root, Environment* env);
void interpret(ASTNode* root);
#endif
