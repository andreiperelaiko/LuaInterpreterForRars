#include "interpreter.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/io.h"
#include "lib/fstream.h"

Table _G = {.capacity = 0, .pairs = 0, .size = 0};
static const Value nil = {.type=VAL_NIL};

static const LValue lvalue_error = {
    .key = {.type = VAL_NIL},
    .table = 0
};

int parse_ll(const char* s){
    int n = 0;
    if (s == 0) return 0;
    while (*s){
        if (*s < '0' || *s > '9'){
            break;
        }
        n = n * 10 + (*s - '0');
        s += 1;
    }
    return n;
}

Value eval(ASTNode* root, Environment* env);
Value eval_number(ASTNode* root, Environment* env);
Value eval_string(ASTNode* root, Environment* env);
Value eval_nil(ASTNode* root, Environment* env);
Value eval_table(ASTNode* root, Environment* env);
Value eval_id(ASTNode* root, Environment* env);
Value eval_binop(ASTNode* root, Environment* env);
Value eval_unop(ASTNode* root, Environment* env);
Value eval_idx(ASTNode* root, Environment* env);
Value eval_call(ASTNode* root, Environment* env);
Value eval_stmt_assign(ASTNode* root, Environment* env);
Value eval_stmt_call(ASTNode* root, Environment* env);
Value eval_stmt_block(ASTNode* root, Environment* env);
Value eval_stmt_if(ASTNode* root, Environment* env);
Value eval_stmt_while(ASTNode* root, Environment* env);
Value eval_stmt_for(ASTNode* root, Environment* env);
Value eval_stmt_for_num(ASTNode* root, Environment* env);
Value eval_stmt_function(ASTNode* root, Environment* env);
Value eval_stmt_return(ASTNode* root, Environment* env);
LValue resolve_lvalue(ASTNode* root, Environment* env);

int valid_lvalue(LValue lvalue){
    if (!lvalue.table){
        return 0;
    }
    if (lvalue.key.type == VAL_NIL){
        return 0;
    }
    return 1;
}
int is_same(Value a, Value b){
    if (a.type != b.type){
        return 0;
    }
    switch (a.type){
        case VAL_NIL:
            return 1;
        case VAL_NUMBER:
            return a.data.number == b.data.number;
        case VAL_STRING:
            return str_eq(a.data.string, b.data.string);
        case VAL_BOOL:
            return a.data.boolean == b.data.boolean;
        case VAL_TABLE:
            return a.data.table == b.data.table;
        default:
            return 0;
    }
}

Table* create_table(){
    Table* table = malloc(sizeof(*table));
    table->capacity = 0;
    table->size = 0;
    table->pairs = 0;
    return table;
}

Environment* create_env(){
    Table* table = create_table();
    Environment* env = malloc(sizeof(*env));
    env->table = table;
    env->parent = 0;
    env->has_return = 0;
    env->return_value = nil;
    return env;
}

int find_slot(Table* table, Value key){
    for (unsigned int i = 0; i < table->size; i++){
        if (is_same(table->pairs[i].first, key)){
            return i;
        }
    }
    return -1;
}

Table* find_table(Environment* env, Value key){
    while (env && env->table && find_slot(env->table, key) == -1){
        env = env->parent;
    }
    if (!env){
        return 0;
    }
    return env->table;
}

void table_set(Table* table, Value key, Value value){
    int pos = find_slot(table, key);
    if (pos >= 0){
        table->pairs[pos].first = key;
        table->pairs[pos].second = value;
        return;
    }
    if (table->size == table->capacity){
        unsigned int old_capacity = table->capacity;
        table->capacity = old_capacity * 2 + 1;
        Pair* tmp = malloc(sizeof(Pair) * table->capacity);
        for (unsigned int i = 0; i < old_capacity; i++){
            tmp[i] = table->pairs[i];
        }
        table->pairs = tmp;
    }
    table->pairs[table->size].first = key;
    table->pairs[table->size].second = value;
    table->size += 1;
}

Value table_get(Table* table, Value key){
    int pos = find_slot(table, key);
    if (pos >= 0) {
        return table->pairs[pos].second;
    }
    return nil;
}

Value env_get(Environment* env, Value key){
    Table* table = find_table(env, key);
    if(!table){
        return nil;
    }
    return table_get(table, key);
}

void env_set(Environment* env, Value key, Value value){
    table_set(env->table, key, value);
}

int is_truthy(Value v){
    if (v.type == VAL_NIL) return 0;
    if (v.type == VAL_BOOL && v.data.boolean == 0) return 0;
    return 1;
}

int as_number(Value v){
    if (v.type != VAL_NUMBER) return 0;
    return v.data.number;
}

Value eval_number(ASTNode* root, Environment* env){
    (void)env;
    Value result = {.type = VAL_NIL};
    if (!root || root->type != NUM){
        return result;
    }
    result.type = VAL_NUMBER;
    result.data.number = parse_ll(root->data.number.value);
    return result;
}

Value eval_string(ASTNode* root, Environment* env){
    (void)env;
    Value result = {.type = VAL_NIL};
    if (!root || root->type != STRING){
        return result;
    }
    result.type = VAL_STRING;
    result.data.string = (char*)root->data.string.value;
    return result;
}

Value eval_nil(ASTNode* root, Environment* env){
    (void)env;
    (void)root;
    Value result = {.type = VAL_NIL};
    return result;
}

Value eval_table(ASTNode* root, Environment* env){
    (void)env;
    Value result = {.type = VAL_NIL};
    if (!root || root->type != TABLE){
        return result;
    }
    Table* table = create_table();
    for (unsigned int i = 0; i < root->data.table_cons.items_cnt; i++){
        Value key = {.type = VAL_NUMBER};
        key.data.number = (int)i + 1;
        Value value = eval(root->data.table_cons.items[i], env);
        table_set(table, key, value);
    }
    result.type = VAL_TABLE;
    result.data.table = table;
    return result;
}

Value eval_id(ASTNode* root, Environment* env){
    Value result = {.type = VAL_NIL};
    if (!root || root->type != ID){
        return result;
    }
    Value key = {.type = VAL_STRING};
    key.data.string = (char*)root->data.ident.value;
    return env_get(env, key);
}

Value eval_binop(ASTNode* root, Environment* env){
    Value nil = {.type = VAL_NIL};
    if (!root) return nil;
    Value lhs = eval(root->data.binop.lhs, env);
    Value rhs = eval(root->data.binop.rhs, env);

    switch (root->type){
        case ADD: {
            Value v = {.type = VAL_NUMBER};
            v.data.number = as_number(lhs) + as_number(rhs);
            return v;
        }
        case SUB: {
            Value v = {.type = VAL_NUMBER};
            v.data.number = as_number(lhs) - as_number(rhs);
            return v;
        }
        case MUL: {
            Value v = {.type = VAL_NUMBER};
            v.data.number = as_number(lhs) * as_number(rhs);
            return v;
        }
        case DIV: {
            Value v = {.type = VAL_NUMBER};
            int r = as_number(rhs);
            v.data.number = (r == 0) ? 0 : (as_number(lhs) / r);
            return v;
        }
        case EQ: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = is_same(lhs, rhs);
            return v;
        }
        case NEQ: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = !is_same(lhs, rhs);
            return v;
        }
        case LT: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = as_number(lhs) < as_number(rhs);
            return v;
        }
        case LEQ: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = as_number(lhs) <= as_number(rhs);
            return v;
        }
        case GT: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = as_number(lhs) > as_number(rhs);
            return v;
        }
        case GEQ: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = as_number(lhs) >= as_number(rhs);
            return v;
        }
        case AND: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = is_truthy(lhs) && is_truthy(rhs);
            return v;
        }
        case OR: {
            Value v = {.type = VAL_BOOL};
            v.data.boolean = is_truthy(lhs) || is_truthy(rhs);
            return v;
        }
        default:
            return nil;
    }
}

Value eval_unop(ASTNode* root, Environment* env){
    Value nil = {.type = VAL_NIL};
    if (!root) return nil;
    Value value = eval((ASTNode*)root->data.unop.value, env);
    if (root->type == NEG){
        Value v = {.type = VAL_NUMBER};
        v.data.number = -as_number(value);
        return v;
    }
    if (root->type == NOT){
        Value v = {.type = VAL_BOOL};
        v.data.boolean = !is_truthy(value);
        return v;
    }
    return nil;
}

Value eval_idx(ASTNode* root, Environment* env){
    Value nil = {.type = VAL_NIL};
    if (!root) return nil;
    Value container = eval(root->data.index.container, env);
    Value index = eval(root->data.index.index, env);
    if (container.type != VAL_TABLE){
        return nil;
    }
    return table_get(container.data.table, index);
}

Value eval_call(ASTNode* root, Environment* env){
    if (!root) return nil;
    
    Value calle = eval(root->data.call.calle, env);
    if (calle.type != VAL_FUNCTION || !calle.data.func){
        return nil;
    }
    Value* args = malloc(sizeof(Value) * root->data.call.args_cnt);
    for (unsigned int i = 0; i < root->data.call.args_cnt; i++){
        args[i] = eval(root->data.call.args[i], env);
    }
    Function* function = calle.data.func;
    if (function->type == C_FUNCTION){
        if (!function->body.c){
            return nil;
        }
        return function->body.c(env, args, root->data.call.args_cnt);
    }
    if (function->type == LUA_FUNCTION){
        Environment call_env = {
            .table = create_table(),
            .parent = function->env,
            .has_return = 0,
            .return_value = nil
        };
        for (unsigned int i = 0; i < function->args_cnt; i++){
            Value key = function->args[i];
            Value value = nil;
            if (i < root->data.call.args_cnt){
                value = args[i];
            }
            env_set(&call_env, key, value);
        }
        Value body_result = eval(function->body.lua, &call_env);
        Value result = call_env.has_return ? call_env.return_value : body_result;
        return result;
    }
    return nil;
}

Value eval_stmt_assign(ASTNode* root, Environment* env){
    if (!root) return nil;
    LValue lhs = resolve_lvalue(root->data.assign_stmt.lhs, env);
    if (!valid_lvalue(lhs)){
        return nil;
    }
    Value rhs = eval(root->data.assign_stmt.rhs, env);
    table_set(lhs.table, lhs.key, rhs);
    return rhs;
}

Value eval_stmt_call(ASTNode* root, Environment* env){
    if (!root){
        Value nil = {.type = VAL_NIL};
        return nil;
    }
    return eval_call(root->data.call_stmt.call, env);
}

Value eval_stmt_block(ASTNode* root, Environment* env){
    Value last = {.type = VAL_NIL};
    if (!root) return last;
    for (unsigned int i = 0; i < root->data.block.stmts_cnt; i++){
        last = eval(root->data.block.stmts[i], env);
        if (env->has_return){
            break;
        }
    }
    return last;
}

Value eval_stmt_if(ASTNode* root, Environment* env){
    Value nil = {.type = VAL_NIL};
    if (!root) return nil;
    Value cond = eval(root->data.if_node.cond, env);
    if (is_truthy(cond)){
        return eval(root->data.if_node.then_block, env);
    }
    if (root->data.if_node.else_block){
        return eval(root->data.if_node.else_block, env);
    }
    return nil;
}

Value eval_stmt_while(ASTNode* root, Environment* env){
    Value last = {.type = VAL_NIL};
    if (!root) return last;
    while (is_truthy(eval(root->data.while_node.cond, env))){
        last = eval(root->data.while_node.body, env);
        if (env->has_return){
            break;
        }
    }
    return last;
}

Value eval_stmt_for(ASTNode* root, Environment *env){
    Value last = {.type = VAL_NIL};
    if (!root) return last;
    Value iterable = eval(root->data.for_node.iterable, env);
    if (iterable.type != VAL_TABLE){
        return last;
    }
    Value key_name = {.type = VAL_STRING};
    key_name.data.string = (char*)root->data.for_node.name;
    for (unsigned int i = 0; i < iterable.data.table->size; i++){
        env_set(env, key_name, iterable.data.table->pairs[i].second);
        last = eval(root->data.for_node.body, env);
        if (env->has_return){
            break;
        }
    }
    return last;
}

Value eval_stmt_for_num(ASTNode* root, Environment *env){
    Value last = {.type = VAL_NIL};
    if (!root) return last;
    Value start_v = eval(root->data.for_num_node.start, env);
    Value stop_v = eval(root->data.for_num_node.stop, env);
    Value step_v = eval(root->data.for_num_node.step, env);
    if (start_v.type != VAL_NUMBER || stop_v.type != VAL_NUMBER || step_v.type != VAL_NUMBER){
        return last;
    }
    int start = start_v.data.number;
    int stop = stop_v.data.number;
    int step = step_v.data.number;
    if (step == 0){
        return last;
    }
    Value key_name = {.type = VAL_STRING};
    key_name.data.string = (char*)root->data.for_num_node.name;
    if (step > 0){
        for (int i = start; i <= stop; i += step){
            Value cur = {.type = VAL_NUMBER};
            cur.data.number = i;
            env_set(env, key_name, cur);
            last = eval(root->data.for_num_node.body, env);
            if (env->has_return){
                break;
            }
        }
    } else {
        for (int i = start; i >= stop; i += step){
            Value cur = {.type = VAL_NUMBER};
            cur.data.number = i;
            env_set(env, key_name, cur);
            last = eval(root->data.for_num_node.body, env);
            if (env->has_return){
                break;
            }
        }
    }
    return last;
}

Value eval_stmt_function(ASTNode* root, Environment* env){
    if (!root) return nil;
    Function* func = malloc(sizeof(*func));
    func->type = LUA_FUNCTION;
    func->env = env;
    func->args_cnt = root->data.func_stmt.params_cnt;
    func->args = malloc(sizeof(Value) * func->args_cnt);
    for (unsigned int i = 0; i < func->args_cnt; i++){
        func->args[i].type = VAL_STRING;
        func->args[i].data.string = root->data.func_stmt.params[i];
    }
    func->body.lua = root->data.func_stmt.body;

    Value key = {.type = VAL_STRING};
    key.data.string = root->data.func_stmt.name;
    Value value = {.type = VAL_FUNCTION};
    value.data.func = func;
    env_set(env, key, value);
    return value;
}

Value eval_stmt_return(ASTNode* root, Environment* env){
    if (!root) return nil;
    Value value = nil;
    if (root->data.return_stmt.value){
        value = eval(root->data.return_stmt.value, env);
    }
    env->has_return = 1;
    env->return_value = value;
    return value;
}

typedef Value (*evaluator)(ASTNode* root, Environment* env);
static evaluator evaluators[AST_NODE_TYPE_COUNT] = {
    [NUM] = eval_number,
    [STRING] = eval_string,
    [NIL] = eval_nil,
    [TABLE] = eval_table,
    [ID] = eval_id,
    [IDX] = eval_idx,
    [CALL] = eval_call,
    [ADD] = eval_binop, [SUB] = eval_binop,
    [MUL] = eval_binop, [DIV] = eval_binop,
    [EXP] = eval_binop, [EQ] = eval_binop,
    [NEQ] = eval_binop, [LT] = eval_binop,
    [LEQ] = eval_binop, [GT] = eval_binop,
    [GEQ] = eval_binop, [AND] = eval_binop,
    [OR] = eval_binop,
    [NEG] = eval_unop, [NOT] = eval_unop,
    [ASSIGN_STMT] = eval_stmt_assign,
    [CALL_STMT] = eval_stmt_call,
    [BLOCK] = eval_stmt_block,
    [IF_STMT] = eval_stmt_if,
    [WHILE_STMT] = eval_stmt_while,
    [FOR_STMT] = eval_stmt_for,
    [FOR_NUM_STMT] = eval_stmt_for_num,
    [FUNC_STMT] = eval_stmt_function,
    [RETURN_STMT] = eval_stmt_return,
};

Value eval(ASTNode* root, Environment* env){
    Value nil = {.type = VAL_NIL};
    if (!root) return nil;
    if (root->type >= AST_NODE_TYPE_COUNT || !evaluators[root->type]){
        return nil;
    }
    return evaluators[root->type](root, env);
}

LValue resolve_idx(ASTNode* root, Environment* env){
    Value container = eval(root->data.index.container, env);
    Value index = eval(root->data.index.index, env);
    if (container.type != VAL_TABLE || index.type == VAL_NIL){
        return lvalue_error;
    }
    
    LValue result = {
        .key = index,
        .table = container.data.table
    };
    return result;
}

LValue resolve_id(ASTNode* root, Environment* env){
    Value id = {.type = VAL_STRING};
    id.data.string = (char*)root->data.ident.value;
    Table* table = find_table(env, id);
    if (!table){
        table = env ? env->table : 0;
    }
    if (!table){
        return lvalue_error;
    }
    LValue result = {
        .key = id,
        .table = table
    };
    return result;
}

typedef LValue (*resolver)(ASTNode* root, Environment* env);
static resolver resolvers[AST_NODE_TYPE_COUNT] = {
    [IDX] = resolve_idx,
    [ID]  = resolve_id,
};

LValue resolve_lvalue(ASTNode* root, Environment* env){
    if (!root) return lvalue_error;
    if(root->type >= AST_NODE_TYPE_COUNT || !resolvers[root->type]){
        return lvalue_error;
    }
    return resolvers[root->type](root, env);
}


Value print(Environment* env, Value* args, unsigned int args_cnt){
    (void)env;
    for (unsigned int i = 0; i < args_cnt; i++){
        Value arg = args[i]; 
        switch(arg.type){
            case VAL_NIL:
                print_string("nil");
                break;
            case VAL_STRING:
                print_string(arg.data.string);
                break;
            case VAL_NUMBER:
                print_int(arg.data.number);
                break;
            default:
                print_string("print error");
                break;
        }
        if (i + 1 < args_cnt){
            print_char(' ');
        }
    }
    print_char('\n');
    return nil;
}

Value copen(Environment* env, Value* args, unsigned int args_cnt){
    (void)env;
    if (args_cnt < 1 || args[0].type != VAL_STRING){
        return nil;
    }
    int fd = open(args[0].data.string);
    Value result = {.type = VAL_NUMBER};
    result.data.number = fd;
    return result;
}

Value cclose(Environment* env, Value* args, unsigned int args_cnt){
    (void)env;
    if (args_cnt < 1 || args[0].type != VAL_NUMBER){
        return nil;
    }
    close(args[0].data.number);
    Value result = {.type = VAL_NUMBER};
    result.data.number = 0;
    return result;
}

Value cread(Environment* env, Value* args, unsigned int args_cnt){
    (void)env;
    if (args_cnt < 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER){
        return nil;
    }
    int fd = args[0].data.number;
    int n = args[1].data.number;
    if (n <= 0){
        Value empty = {.type = VAL_STRING};
        empty.data.string = "";
        return empty;
    }
    char* buffer = malloc((unsigned int)n + 1);
    int got = read(fd, buffer, n);
    if (got < 0){
        return nil;
    }
    buffer[got] = '\0';
    Value result = {.type = VAL_STRING};
    result.data.string = buffer;
    return result;
}

Value cwrite(Environment* env, Value* args, unsigned int args_cnt){
    (void)env;
    if (args_cnt < 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_STRING){
        return nil;
    }
    int fd = args[0].data.number;
    char* text = (char*)args[1].data.string;
    int len = strlen(text);
    int wrote = write(fd, text, len);
    Value result = {.type = VAL_NUMBER};
    result.data.number = wrote;
    return result;
}

Value clseek(Environment* env, Value* args, unsigned int args_cnt){
    (void)env;
    if (args_cnt < 3 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER || args[2].type != VAL_NUMBER){
        return nil;
    }
    int fd = args[0].data.number;
    int off = args[1].data.number;
    int whence = args[2].data.number;
    int pos = lseek(fd, off, whence);
    Value result = {.type = VAL_NUMBER};
    result.data.number = pos;
    return result;
}

void register_cfunc(Environment* env, const char* name, c_func raw_func){
    Value key = {
        .type = VAL_STRING,
        .data = {
            .string = name
        }
    };
    Function *func = malloc(sizeof(*func));
    func->type = C_FUNCTION;
    func->body.c = raw_func;
    Value value = {
        .type = VAL_FUNCTION,
        .data = {
            .func = func
        }
    };;
    env_set(env, key, value);
}

void interpret(ASTNode* root){
    Environment env = {.parent = 0, .table = &_G, .has_return = 0, .return_value = {.type = VAL_NIL}};
    register_cfunc(&env, "print", print);
    register_cfunc(&env, "open", copen);
    register_cfunc(&env, "close", cclose);
    register_cfunc(&env, "read", cread);
    register_cfunc(&env, "write", cwrite);
    register_cfunc(&env, "lseek", clseek);
    (void)eval(root, &env);
}