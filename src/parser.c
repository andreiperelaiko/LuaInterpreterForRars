#include "parser.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/test.h"
#include "tokenizer.h"
#include "ast.h"

static int precedences[] = {
	[TokOr] = 1,    [TokAnd] = 2,
	[TokEqual] = 3,  [TokNotequal] = 3,
	[TokGEq] = 3,    [TokLEq] = 3,
	[TokGT] = 3,     [TokLT] = 3,
	[TokDDot] = 4,
	[TokMinus] = 5,  [TokPlus] = 5,
	[TokSlash] = 6,  [TokStar] = 6,
	[TokExp] = 7,
};

static ASTNodeType binop_types[] = {
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

ASTNode *parse_expr(TokStream *stream, int min_precedence) {
	ASTNode *left = parse_nud(stream);
	while (!ts_eof(stream)) {
		Token token = ts_peek(stream);
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
	if (!ts_eof(stream) && ts_peek(stream).type != TokRParen) {
		args[args_cnt++] = parse_expr(stream, 0);
		while (!ts_eof(stream) && ts_peek(stream).type == TokComma) {
			ts_get(stream);
			args[args_cnt++] = parse_expr(stream, 0);
		}
	}
	call->data.call.args_cnt = args_cnt;
	call->data.call.args = args;
}

ASTNode *suffix_dot(TokStream *stream, ASTNode *container) {
	ts_get(stream);
	if (ts_eof(stream) || ts_peek(stream).type != TokIdent)
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
	ts_get(stream);
	ASTNode *index = parse_expr(stream, 0);
	if (ts_eof(stream) || ts_peek(stream).type != TokRBracket)
		return 0;
	ts_get(stream);
	ASTNode *node = malloc(sizeof(*node));
	node->type = IDX;
	node->data.index.index = index;
	node->data.index.container = container;
	return node;
}

ASTNode *suffix_call(TokStream *stream, ASTNode *container) {
	ts_get(stream);
	ASTNode *call = malloc(sizeof(*call));
	call->type = CALL;
	call->data.call.calle = container;
	parse_args(stream, call);
	if (ts_eof(stream) || ts_peek(stream).type != TokRParen)
		return 0;
	ts_get(stream);
	return call;
}

typedef ASTNode *(*suffix_parser)(TokStream *stream, ASTNode *container);

static suffix_parser suffix_parsers[] = {
	[TokDot] = suffix_dot,
	[TokLBracket] = suffix_bracket,
	[TokLParen] = suffix_call,
};

#define SUFFIX_COUNT (sizeof(suffix_parsers) / sizeof(suffix_parsers[0]))

ASTNode *parse_ident(TokStream *stream) {
	Token container = ts_peek(stream);
	if (container.type != TokIdent)
		return 0;
	ts_get(stream);
	ASTNode *node = malloc(sizeof(*node));
	node->type = ID;
	node->data.ident.value = container.value;

	while (!ts_eof(stream)) {
		TokenType suff = ts_peek(stream).type;
		if (suff >= SUFFIX_COUNT || !suffix_parsers[suff])
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

ASTNode *nud_paren(TokStream *stream) {
	ts_get(stream);
	ASTNode *expr = parse_expr(stream, 0);
	if (ts_eof(stream) || ts_peek(stream).type != TokRParen)
		return 0;
	ts_get(stream);
	return expr;
}

typedef ASTNode *(*nud_parser)(TokStream *stream);

static nud_parser nud_parsers[] = {
	[TokInt] = nud_int,
	[TokIdent] = parse_ident,
	[TokString] = nud_string,
	[TokLParen] = nud_paren,
};

#define NUD_COUNT (sizeof(nud_parsers) / sizeof(nud_parsers[0]))

ASTNode *parse_nud(TokStream *stream) {
	TokenType type = ts_peek(stream).type;
	if (type < NUD_COUNT && nud_parsers[type])
		return nud_parsers[type](stream);
	return 0;
}

