#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include "lib/string.h"
#include "lib/memory.h"
#include "lib/fstream.h"

void print_int(int n);
void print_char(char c);
void print_string(const char* s);

#ifndef LUA_SCRIPT_PATH
#define LUA_SCRIPT_PATH "tests/interpreter/print.lua"
#endif

int main() {
    const char* script_path = LUA_SCRIPT_PATH;
    int fd = open(script_path);
    if (fd < 0) {
        print_string("open failed: ");
        print_string(script_path);
        print_char('\n');
        return 1;
    }

    int file_size = lseek(fd, 0, 2);
    if (file_size < 0) {
        print_string("lseek failed\n");
        close(fd);
        return 1;
    }
    lseek(fd, 0, 0);

    char* code = malloc((unsigned int)file_size + 1);
    int read_size = read(fd, code, file_size);
    close(fd);
    if (read_size < 0) {
        print_string("read failed\n");
        return 1;
    }
    code[read_size] = '\0';

    CharStream c_stream = {
        .buffer = code,
        .cursor = 0,
        .size = strlen(code)
    };
    VecTokens tokens = tokenize(&c_stream);
    TokStream t_stream = make_tokstream(&tokens);
    ASTNode* root = parse_block(&t_stream);
    interpret(root);

    return 0;
}
