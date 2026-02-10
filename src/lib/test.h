#ifndef TEST_H
#define TEST_H

#include "lib/io.h"
#include "lib/string.h"

typedef struct {
    int passed;
    int failed;
    const char* current;
} TestCtx;

void eq_int(TestCtx* t, int got, int expected);
void eq_str(TestCtx* t, const char* got, const char* expected);
void test_summary(TestCtx* t);

#define ASSERT_INT(got, exp) eq_int(_t, got, exp)
#define ASSERT_STR(got, exp) eq_str(_t, got, exp)
#define TEST(fn) void fn(TestCtx* _t)
#define RUN(fn) do { _t.current = #fn; fn(&_t); } while(0)

#endif
