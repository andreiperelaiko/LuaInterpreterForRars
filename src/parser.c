#include "parser.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/test.h"
#include "tokenizer.h"
#include "ast.h"

static int precedences[TOKEN_TYPE_COUNT] = {
	[TokOr] = 1,    [TokAnd] = 2,
	[TokEqual] = 3,  [TokNotequal] = 3,
	[TokGEq] = 3,    [TokLEq] = 3,
	[TokGT] = 3,     [TokLT] = 3,
	[TokDDot] = 4,
	[TokMinus] = 5,  [TokPlus] = 5,
	[TokSlash] = 6,  [TokStar] = 6,
	[TokExp] = 7,
};

static ASTNodeType binop_types[TOKEN_TYPE_COUNT] = {
	[TokPlus] = ADD,  [TokMinus] = SUB,
	[TokStar] = MUL,  [TokSlash] = DIV,
	[TokExp] = EXP,   [TokEqual] = EQ,
	[TokNotequal] = NEQ, [TokLEq] = LEQ,
	[TokGEq] = GEQ,   [TokLT] = LT,
	[TokGT] = GT,     [TokAnd] = AND,
	[TokOr] = OR,
};

int is_operator(TokenType type) { return precedences[type] > 0; }
int get_precedence(TokenType type) { return precedences[type]; }
ASTNodeType convert_binop(TokenType type) { return binop_types[type]; }

int tok_is(TokStream *stream, TokenType expected) {
	return ts_peek(stream).type == expected;
}

int try_consume(TokStream *stream, TokenType expected) {
	if (!tok_is(stream, expected)) {
		return 0;
	}
	ts_get(stream);
	return 1;
}

ASTNode *parse_expr(TokStream *stream, int min_precedence) {
	ASTNode *left = parse_nud(stream);
	while (left) {
		Token token = ts_peek(stream);
		if (token.type == TokError) {
			break;
		}
		if (!is_operator(token.type)) {
			break;
		}
		int precedence = get_precedence(token.type);
		if (precedence < min_precedence) {
			break;
		}
		ts_get(stream);
		ASTNode *right = parse_expr(stream, precedence + 1);
		ASTNode *node = malloc(sizeof(ASTNode));
		node->type = convert_binop(token.type);
		node->data.binop.lhs = left;
		node->data.binop.rhs = right;
		left = node;
	}
	return left;
}

void parse_args(TokStream *stream, ASTNode *call) {
	ASTNode **args = malloc(sizeof(ASTNode *) * MAX_CALL_ARGS);
	unsigned int args_cnt = 0;
	if (!tok_is(stream, TokRParen)) {
		args[args_cnt++] = parse_expr(stream, 0);
		while (try_consume(stream, TokComma)) {
			args[args_cnt++] = parse_expr(stream, 0);
		}
	}
	call->data.call.args_cnt = args_cnt;
	call->data.call.args = args;
}

ASTNode *suffix_dot(TokStream *stream, ASTNode *container) {
	try_consume(stream, TokDot);
	if (!tok_is(stream, TokIdent))
		return 0;
	Token id = ts_get(stream);
	ASTNode *key = malloc(sizeof(*key));
	key->type = ID;
	key->data.ident.value = id.value;
	ASTNode *node = malloc(sizeof(*node));
	node->type = IDX;
	node->data.index.index = key;
	node->data.index.container = container;
	return node;
}

ASTNode *suffix_bracket(TokStream *stream, ASTNode *container) {
	try_consume(stream, TokLBracket);
	ASTNode *index = parse_expr(stream, 0);
	if (!try_consume(stream, TokRBracket))
		return 0;
	ASTNode *node = malloc(sizeof(*node));
	node->type = IDX;
	node->data.index.index = index;
	node->data.index.container = container;
	return node;
}

ASTNode *suffix_call(TokStream *stream, ASTNode *container) {
	try_consume(stream, TokLParen);
	ASTNode *call = malloc(sizeof(*call));
	call->type = CALL;
	call->data.call.calle = container;
	parse_args(stream, call);
	if (!try_consume(stream, TokRParen))
		return 0;
	return call;
}

typedef ASTNode *(*suffix_parser)(TokStream *stream, ASTNode *container);

static suffix_parser suffix_parsers[TOKEN_TYPE_COUNT] = {
	[TokDot] = suffix_dot,
	[TokLBracket] = suffix_bracket,
	[TokLParen] = suffix_call,
};

ASTNode *parse_ident(TokStream *stream) {
	Token container = ts_peek(stream);
	if (container.type != TokIdent)
		return 0;
	ts_get(stream);
	ASTNode *node = malloc(sizeof(*node));
	node->type = ID;
	node->data.ident.value = container.value;

	while (1) {
		TokenType suff = ts_peek(stream).type;
		if (suff == TokError)
			break;
		if (suff >= TOKEN_TYPE_COUNT || !suffix_parsers[suff])
			break;
		node = suffix_parsers[suff](stream, node);
		if (!node)
			return 0;
	}
	return node;
}

ASTNode *nud_int(TokStream *stream) {
	Token token = ts_get(stream);
	ASTNode *node = malloc(sizeof(*node));
	node->type = NUM;
	node->data.number.value = token.value;
	return node;
}

ASTNode *nud_string(TokStream *stream) {
	Token token = ts_get(stream);
	ASTNode *node = malloc(sizeof(*node));
	node->type = STRING;
	node->data.string.value = token.value;
	return node;
}

ASTNode *nud_nil(TokStream *stream) {
	ts_get(stream);
	ASTNode *node = malloc(sizeof(*node));
	node->type = NIL;
	return node;
}

ASTNode* nud_not(TokStream* stream){
	ts_get(stream);
	ASTNode* node = malloc(sizeof(*node));
	ASTNode* expr = parse_nud(stream);
	node->type = NOT;
	node->data.unop.value = expr;
	return node;
}

ASTNode* nud_neg(TokStream* stream){
	ts_get(stream);
	ASTNode* node = malloc(sizeof(*node));
	ASTNode* expr = parse_nud(stream);
	node->type = NEG;
	node->data.unop.value = expr;
	return node;
}

ASTNode *nud_paren(TokStream *stream) {
	try_consume(stream, TokLParen);
	ASTNode *expr = parse_expr(stream, 0);
	if (!try_consume(stream, TokRParen))
		return 0;
	return expr;
}

typedef ASTNode *(*nud_parser)(TokStream *stream);

static nud_parser nud_parsers[TOKEN_TYPE_COUNT] = {
	[TokInt] = nud_int,
	[TokIdent] = parse_ident,
	[TokString] = nud_string,
	[TokNil] = nud_nil,
	[TokLParen] = nud_paren,
	[TokNot] = nud_not,
	[TokMinus] = nud_neg,
};

ASTNode *parse_nud(TokStream *stream) {
	TokenType type = ts_peek(stream).type;
	if (type < TOKEN_TYPE_COUNT && nud_parsers[type])
		return nud_parsers[type](stream);
	return 0;
}

ASTNode *stmt_if(TokStream *stream) {
	try_consume(stream, TokIf);
	ASTNode *cond = parse_expr(stream, 0);
	if (!cond || !try_consume(stream, TokThen))
		return 0;

	ASTNode *then_block = parse_block(stream);
	ASTNode *else_block = 0;

	if (try_consume(stream, TokElse)) {
		else_block = parse_block(stream);
	}

	if (!try_consume(stream, TokEnd))
		return 0;

	ASTNode *node = malloc(sizeof(*node));
	node->type = IF_STMT;
	node->data.if_node.cond = cond;
	node->data.if_node.then_block = then_block;
	node->data.if_node.else_block = else_block;
	return node;
}

ASTNode *stmt_while(TokStream *stream) {
	try_consume(stream, TokWhile);
	ASTNode *cond = parse_expr(stream, 0);
	if (!cond || !try_consume(stream, TokDo))
		return 0;
	ASTNode *body = parse_block(stream);
	if (!body || !try_consume(stream, TokEnd))
		return 0;
	ASTNode *node = malloc(sizeof(*node));
	node->type = WHILE_STMT;
	node->data.while_node.cond = cond;
	node->data.while_node.body = body;
	return node;
}

ASTNode *stmt_for(TokStream *stream) {
	try_consume(stream, TokFor);
	if (!tok_is(stream, TokIdent))
		return 0;
	Token var = ts_get(stream);
	if (!try_consume(stream, TokIn))
		return 0;
	ASTNode *iterable = parse_expr(stream, 0);
	if (!iterable || !try_consume(stream, TokDo))
		return 0;
	ASTNode *body = parse_block(stream);
	if (!body || !try_consume(stream, TokEnd))
		return 0;
	ASTNode *node = malloc(sizeof(*node));
	node->type = FOR_STMT;
	node->data.for_node.name = var.value;
	node->data.for_node.iterable = iterable;
	node->data.for_node.body = body;
	return node;
}

ASTNode *stmt_call_or_assign(TokStream *stream) {
	ASTNode *left = parse_ident(stream);
	if (!left)
		return 0;
	if (try_consume(stream, TokAssign)) {
		ASTNode *rhs = parse_expr(stream, 0);
		if (!rhs)
			return 0;
		ASTNode *node = malloc(sizeof(*node));
		node->type = ASSIGN_STMT;
		node->data.assign_stmt.lhs = left;
		node->data.assign_stmt.rhs = rhs;
		return node;
	}
	if (left->type == CALL) {
		ASTNode *node = malloc(sizeof(*node));
		node->type = CALL_STMT;
		node->data.call_stmt.call = left;
		return node;
	}
	return 0;
}

ASTNode *stmt_function(TokStream *stream) {
	if (!try_consume(stream, TokFunction))
		return 0;
	if (!tok_is(stream, TokIdent))
		return 0;
	Token fn = ts_get(stream);
	if (!try_consume(stream, TokLParen))
		return 0;
	const char **params = malloc(sizeof(const char *) * MAX_FUNC_PARAMS);
	unsigned int params_cnt = 0;
	if (!tok_is(stream, TokRParen)) {
		if (!tok_is(stream, TokIdent))
			return 0;
		params[params_cnt++] = ts_get(stream).value;
		while (try_consume(stream, TokComma)) {
			if (!tok_is(stream, TokIdent))
				return 0;
			params[params_cnt++] = ts_get(stream).value;
		}
	}
	if (!try_consume(stream, TokRParen))
		return 0;
	ASTNode *body = parse_block(stream);
	if (!body || !try_consume(stream, TokEnd))
		return 0;
	ASTNode *node = malloc(sizeof(*node));
	node->type = FUNC_STMT;
	node->data.func_stmt.name = fn.value;
	node->data.func_stmt.params = params;
	node->data.func_stmt.params_cnt = params_cnt;
	node->data.func_stmt.body = body;
	return node;
}

typedef ASTNode* (*stmt_parser)(TokStream* stream);
static stmt_parser stmts_parsers[TOKEN_TYPE_COUNT] = {
	[TokIdent] = stmt_call_or_assign,
	[TokFunction] = stmt_function,
	[TokFor] = stmt_for,
	[TokWhile] = stmt_while,
	[TokIf] = stmt_if,
};

ASTNode *parse_stmt(TokStream *stream) {
	TokenType type = ts_peek(stream).type;
	if(type >= TOKEN_TYPE_COUNT || !stmts_parsers[type]){
		return 0;
	}
	return stmts_parsers[type](stream);	
}

ASTNode *parse_block(TokStream *stream) {
	ASTNode **stmts = malloc(sizeof(ASTNode *) * MAX_BLOCK_STMTS);
	unsigned int cnt = 0;
	while (1) {
		TokenType t = ts_peek(stream).type;
		if (t == TokError || t == TokElse || t == TokEnd)
			break;
		ASTNode *s = parse_stmt(stream);
		if (!s) break;
		stmts[cnt++] = s;
	}
	ASTNode *node = malloc(sizeof(*node));
	node->type = BLOCK;
	node->data.block.stmts = stmts;
	node->data.block.stmts_cnt = cnt;
	return node;
}

