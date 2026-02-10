#include "lib/test.h"
#include "tokenizer.h"

CharStream make_stream(const char* s){
    CharStream cs = {.buffer = s, .cursor = 0, .size = strlen(s)};
    return cs;
}

TEST(test_cs_peek){
    CharStream s = make_stream("abc");
    ASSERT_INT(cs_peek(&s), 'a');
    ASSERT_INT(cs_peek(&s), 'a');
}

TEST(test_cs_get){
    CharStream s = make_stream("abc");
    ASSERT_INT(cs_get(&s), 'a');
    ASSERT_INT(cs_peek(&s), 'b');
}

TEST(test_cs_eof){
    CharStream s = make_stream("a");
    ASSERT_INT(cs_eof(&s), 0);
    cs_get(&s);
    ASSERT_INT(cs_eof(&s), 1);
}

TEST(test_skip_whitespace){
    CharStream s1 = make_stream("   x");
    skipWhitespace(&s1);
    ASSERT_INT(cs_peek(&s1), 'x');

    CharStream s2 = make_stream("\n\nx");
    skipWhitespace(&s2);
    ASSERT_INT(cs_peek(&s2), 'x');
}

TEST(test_parse_int){
    CharStream s1 = make_stream("123");
    Token t1 = parseTokInt(&s1);
    ASSERT_INT(t1.type, TokInt);
    ASSERT_STR(t1.value, "123");

    CharStream s2 = make_stream("abc");
    Token t2 = parseTokInt(&s2);
    ASSERT_INT(t2.type, TokError);

    CharStream s3 = make_stream("42+");
    Token t3 = parseTokInt(&s3);
    ASSERT_STR(t3.value, "42");
    ASSERT_INT(cs_peek(&s3), '+');
}

TEST(test_parse_plus){
    CharStream s1 = make_stream("+");
    Token t1 = parseTokPlus(&s1);
    ASSERT_INT(t1.type, TokPlus);

    CharStream s2 = make_stream("x");
    Token t2 = parseTokPlus(&s2);
    ASSERT_INT(t2.type, TokError);
}

TEST(test_parse_star){
    CharStream s1 = make_stream("*");
    Token t1 = parseTokStar(&s1);
    ASSERT_INT(t1.type, TokStar);

    CharStream s2 = make_stream("x");
    Token t2 = parseTokStar(&s2);
    ASSERT_INT(t2.type, TokError);
}

TEST(test_tokenize_simple){
    CharStream s = make_stream("10 + 20");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 3);
    ASSERT_INT(get_token(&tokens, 0).type, TokInt);
    ASSERT_STR(get_token(&tokens, 0).value, "10");
    ASSERT_INT(get_token(&tokens, 1).type, TokPlus);
    ASSERT_INT(get_token(&tokens, 2).type, TokInt);
    ASSERT_STR(get_token(&tokens, 2).value, "20");
}

TEST(test_tokenize_mixed){
    CharStream s = make_stream("1+2*3");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 5);
    ASSERT_INT(get_token(&tokens, 1).type, TokPlus);
    ASSERT_INT(get_token(&tokens, 3).type, TokStar);
}

TEST(test_tokenize_spaces){
    CharStream s = make_stream("1   +   2");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 3);
    ASSERT_STR(get_token(&tokens, 0).value, "1");
    ASSERT_STR(get_token(&tokens, 2).value, "2");
}

int main(){
    TestCtx _t = {.passed = 0, .failed = 0, .current = 0};

    RUN(test_cs_peek);
    RUN(test_cs_get);
    RUN(test_cs_eof);
    RUN(test_skip_whitespace);
    RUN(test_parse_int);
    RUN(test_parse_plus);
    RUN(test_parse_star);
    RUN(test_tokenize_simple);
    RUN(test_tokenize_mixed);
    RUN(test_tokenize_spaces);

    test_summary(&_t);
    return _t.failed;
}
