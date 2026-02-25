#include "tokenizer.h"
#include "ast.h"



ASTNodeType convert(TokenType type);
int is_operator(TokenType type);

int get_precedence(TokenType type);
ASTNode* parse_expr(TokStream* stream, int precedence);
ASTNode* parse_nud(TokStream* stream);
ASTNode* parse_stmt(TokStream* stream);
ASTNode* parse_block(TokStream* stream);