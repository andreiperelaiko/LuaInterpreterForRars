#include "lib/test.h"
#include "lib/io.h"
#include "lib/memory.h"
#include "lib/fstream.h"
#include "parser.h"

#define MAX_TEST_SIZE (10 * 1024)
void load_test(const char* testname, char* lua_buffer, char* json_buffer){
    const char* dir = "tests/parser/fixtures/";
    const char* lua_path = str_concat(str_concat(dir, testname), ".lua");
    const char* json_path = str_concat(str_concat(dir, testname), ".json");
    
    int lua_fd = open(lua_path);
    int lua_len = read(lua_fd, lua_buffer, MAX_TEST_SIZE - 1);
    if (lua_len > 0) lua_buffer[lua_len] = '\0';
    else lua_buffer[0] = '\0';
    close(lua_fd);

    int json_fd = open(json_path);
    int json_len = read(json_fd, json_buffer, MAX_TEST_SIZE - 1);
    if (json_len > 0) json_buffer[json_len] = '\0';
    else json_buffer[0] = '\0';
    close(json_fd);
}

TokStream get_t_stream(const char* lua_code){
    CharStream c_stream = {lua_code, 0, strlen(lua_code)};
    VecTokens tokens = tokenize(&c_stream);
    TokStream t_stream = {tokens.data, 0, tokens.size};
    return t_stream;    
}

TEST(test_expr){
    const char* cases[] = {"num", "nil", "table_empty", "table_values", "sum", "mul", "long_sum", "sum_mul_precedence", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_idx){
    const char* cases[] = {"dot_access", "bracket_access", "nested_dot", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_call){
    const char* cases[] = {"simple_call", "call_args", "call_expr_arg", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_binops){
    const char* cases[] = {"sub", "div", "exp", "eq", "neq", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_unop){
    const char* cases[] = {"neg", "not", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);
    
    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_precedence){
    const char* cases[] = {"and_or_prec", "cmp_and", "exp_mul_prec", "eq_or", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_if){
    const char* cases[] = {"if_simple", "if_else", "if_expr", "if_nested", "if_stmt_body", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_stmt(&stream);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_loops){
    const char* cases[] = {"while_simple", "while_if", "for_simple", "for_while", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_stmt(&stream);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_stmt_call_assign){
    const char* cases[] = {"stmt_call", "stmt_assign", "stmt_assign_nil", "stmt_assign_expr", "stmt_assign_index", "stmt_call_chain", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_stmt(&stream);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_stmt_function){
    const char* cases[] = {"stmt_func_empty", "stmt_func_params", "stmt_func_body", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_stmt(&stream);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_block){
    const char* cases[] = {"block_mixed", "block_func_loop", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_block(&stream);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

TEST(test_nested){
    const char* cases[] = {"method_call", "chain", "nested_calls", 0};
    char* lua_buf = malloc(MAX_TEST_SIZE);
    char* json_buf = malloc(MAX_TEST_SIZE);

    for (int i = 0; cases[i]; i++){
        load_test(cases[i], lua_buf, json_buf);
        ASTNode* expected = ast_load(json_buf);
        TokStream stream = get_t_stream(lua_buf);
        ASTNode* got = parse_expr(&stream, 0);
        ASSERT_STR(ast_dump(got), ast_dump(expected));
    }
}

int main(){
    TestCtx _t = {.passed = 0, .failed = 0, .current = 0};
    RUN(test_expr);
    RUN(test_unop);
    RUN(test_binops);
    RUN(test_precedence);
    RUN(test_idx);
    RUN(test_call);
    RUN(test_nested);
    RUN(test_if);
    RUN(test_loops);
    RUN(test_stmt_call_assign);
    RUN(test_stmt_function);
    RUN(test_block);
    
    test_summary(&_t);
    return _t.failed;
}