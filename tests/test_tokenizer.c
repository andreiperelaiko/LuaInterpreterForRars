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

TEST(test_parse_minus){
    CharStream s1 = make_stream("-");
    ASSERT_INT(parseTokMinus(&s1).type, TokMinus);

    CharStream s2 = make_stream("x");
    ASSERT_INT(parseTokMinus(&s2).type, TokError);
}

TEST(test_parse_slash){
    CharStream s1 = make_stream("/");
    ASSERT_INT(parseTokSlash(&s1).type, TokSlash);

    CharStream s2 = make_stream("x");
    ASSERT_INT(parseTokSlash(&s2).type, TokError);
}

TEST(test_parse_exp){
    CharStream s1 = make_stream("^");
    ASSERT_INT(parseTokExpr(&s1).type, TokExp);

    CharStream s2 = make_stream("x");
    ASSERT_INT(parseTokExpr(&s2).type, TokError);
}

TEST(test_parse_equal){
    CharStream s1 = make_stream("==");
    ASSERT_INT(parseTokEqual(&s1).type, TokEqual);

    CharStream s2 = make_stream("=x");
    ASSERT_INT(parseTokEqual(&s2).type, TokError);
    ASSERT_INT(cs_peek(&s2), '=');
}

TEST(test_parse_notequal){
    CharStream s1 = make_stream("~=");
    ASSERT_INT(parseTokNotequal(&s1).type, TokNotequal);

    CharStream s2 = make_stream("~x");
    ASSERT_INT(parseTokNotequal(&s2).type, TokError);
    ASSERT_INT(cs_peek(&s2), '~');
}

TEST(test_parse_leq_geq){
    CharStream s1 = make_stream("<=");
    ASSERT_INT(parseTokLEq(&s1).type, TokLEq);

    CharStream s2 = make_stream(">=");
    ASSERT_INT(parseTokGEq(&s2).type, TokGEq);

    CharStream s3 = make_stream("<x");
    ASSERT_INT(parseTokLEq(&s3).type, TokError);
    ASSERT_INT(cs_peek(&s3), '<');
}

TEST(test_parse_lt_gt){
    CharStream s1 = make_stream("<");
    ASSERT_INT(parseTokLT(&s1).type, TokLT);

    CharStream s2 = make_stream(">");
    ASSERT_INT(parseTokGT(&s2).type, TokGT);
}

TEST(test_parse_ddot){
    CharStream s1 = make_stream("..");
    ASSERT_INT(parseTokDDot(&s1).type, TokDDot);

    CharStream s2 = make_stream(".x");
    ASSERT_INT(parseTokDDot(&s2).type, TokError);
    ASSERT_INT(cs_peek(&s2), '.');
}

TEST(test_parse_and_or){
    CharStream s1 = make_stream("and ");
    ASSERT_INT(parseTokAnd(&s1).type, TokAnd);

    CharStream s2 = make_stream("or ");
    ASSERT_INT(parseTokOr(&s2).type, TokOr);

    CharStream s3 = make_stream("android");
    ASSERT_INT(parseTokAnd(&s3).type, TokError);
    ASSERT_INT(cs_peek(&s3), 'a');

    CharStream s4 = make_stream("order");
    ASSERT_INT(parseTokOr(&s4).type, TokError);
    ASSERT_INT(cs_peek(&s4), 'o');
}

TEST(test_parse_ident){
    CharStream s1 = make_stream("foo");
    Token t1 = parseTokIdent(&s1);
    ASSERT_INT(t1.type, TokIdent);
    ASSERT_STR(t1.value, "foo");

    CharStream s2 = make_stream("123");
    ASSERT_INT(parseTokIdent(&s2).type, TokError);
}

TEST(test_tokenize_all_ops){
    CharStream s = make_stream("1 + 2 - 3 * 4 / 5 ^ 6");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 11);
    ASSERT_INT(get_token(&tokens, 1).type, TokPlus);
    ASSERT_INT(get_token(&tokens, 3).type, TokMinus);
    ASSERT_INT(get_token(&tokens, 5).type, TokStar);
    ASSERT_INT(get_token(&tokens, 7).type, TokSlash);
    ASSERT_INT(get_token(&tokens, 9).type, TokExp);
}

TEST(test_tokenize_comparison){
    CharStream s = make_stream("a == b ~= c <= d >= e < f > g");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 13);
    ASSERT_INT(get_token(&tokens, 1).type, TokEqual);
    ASSERT_INT(get_token(&tokens, 3).type, TokNotequal);
    ASSERT_INT(get_token(&tokens, 5).type, TokLEq);
    ASSERT_INT(get_token(&tokens, 7).type, TokGEq);
    ASSERT_INT(get_token(&tokens, 9).type, TokLT);
    ASSERT_INT(get_token(&tokens, 11).type, TokGT);
}

TEST(test_tokenize_keywords){
    CharStream s = make_stream("a and b or c");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 5);
    ASSERT_INT(get_token(&tokens, 0).type, TokIdent);
    ASSERT_INT(get_token(&tokens, 1).type, TokAnd);
    ASSERT_INT(get_token(&tokens, 2).type, TokIdent);
    ASSERT_INT(get_token(&tokens, 3).type, TokOr);
    ASSERT_INT(get_token(&tokens, 4).type, TokIdent);
}

TEST(test_tokenize_ddot){
    CharStream s = make_stream("a .. b");
    VecTokens tokens = tokenize(&s);
    ASSERT_INT(tokens.size, 3);
    ASSERT_INT(get_token(&tokens, 0).type, TokIdent);
    ASSERT_INT(get_token(&tokens, 1).type, TokDDot);
    ASSERT_INT(get_token(&tokens, 2).type, TokIdent);
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
    RUN(test_parse_minus);
    RUN(test_parse_slash);
    RUN(test_parse_exp);
    RUN(test_parse_equal);
    RUN(test_parse_notequal);
    RUN(test_parse_leq_geq);
    RUN(test_parse_lt_gt);
    RUN(test_parse_ddot);
    RUN(test_parse_and_or);
    RUN(test_parse_ident);
    RUN(test_tokenize_simple);
    RUN(test_tokenize_mixed);
    RUN(test_tokenize_spaces);
    RUN(test_tokenize_all_ops);
    RUN(test_tokenize_comparison);
    RUN(test_tokenize_keywords);
    RUN(test_tokenize_ddot);

    test_summary(&_t);
    return _t.failed;
}
