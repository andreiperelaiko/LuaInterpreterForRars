#include "memory.h"
#include "tokenizer.h"

void print_int(int n);
void print_char(char c);
void print_string(const char* s);

int main() {
    const char* code = "10 + 20 + 30";
    
    CharStream stream = {
        .buffer = code,
        .cursor = 0,
        .size = 12
    };
    
    VecTokens tokens = tokenize(&stream);
    
    print_string("Tokens count: ");
    print_int(tokens.size);
    print_char('\n');
    
    for (unsigned int i = 0; i < tokens.size; i++) {
        Token tok = get_token(&tokens, i);
        if (tok.type == TokInt) {
            print_string("INT: ");
            print_string(tok.value);
        } else if (tok.type == TokPlus) {
            print_string("PLUS");
        }
        print_char('\n');
    }
    
    return 0;
}
