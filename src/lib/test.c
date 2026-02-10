#include "lib/test.h"

void eq_int(TestCtx* t, int got, int expected){
    if (got == expected){
        t->passed++;
    } else {
        print_string("FAIL ");
        print_string(t->current);
        print_string(": got ");
        print_int(got);
        print_string(", expected ");
        print_int(expected);
        print_char('\n');
        t->failed++;
    }
}

void eq_str(TestCtx* t, const char* got, const char* expected){
    if (str_eq(got, expected)){
        t->passed++;
    } else {
        print_string("FAIL ");
        print_string(t->current);
        print_string(": got \"");
        if (got) print_string(got);
        print_string("\", expected \"");
        if (expected) print_string(expected);
        print_string("\"\n");
        t->failed++;
    }
}

void test_summary(TestCtx* t){
    print_string("\nPassed: ");
    print_int(t->passed);
    print_string("\nFailed: ");
    print_int(t->failed);
    print_char('\n');
}
