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
    const char* cases[] = {"num", "sum", "mul", "long_sum", "sum_mul_precedence", 0};
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
    
    test_summary(&_t);
    return _t.failed;
}