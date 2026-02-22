#include "ast.h"
#include "lib/char_stream.h"
#include "tokenizer.h"
#include "lib/memory.h"
#include "lib/string.h"


static const char *type_names[] = {
	[ADD] = "ADD", [MUL] = "MUL", [DIV] = "DIV", [SUB] = "SUB",
	[EXP] = "EXP", [EQ] = "EQ",   [NEQ] = "NEQ", [LEQ] = "LEQ",
	[GEQ] = "GEQ", [LT] = "LT",   [GT] = "GT",   [AND] = "AND",
	[OR] = "OR",   [NUM] = "NUM",  [ID] = "ID",   [STRING] = "STRING",
	[IDX] = "IDX", [CALL] = "CALL",
};

#define TYPE_NAMES_COUNT (sizeof(type_names) / sizeof(type_names[0]))

const char *ast_type_name(ASTNodeType type) {
	if (type < TYPE_NAMES_COUNT && type_names[type])
		return type_names[type];
	return "UNKNOWN";
}

int is_binop_type(ASTNodeType type) {
	switch (type) {
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case EXP:
	case EQ:
	case NEQ:
	case LEQ:
	case GEQ:
	case LT:
	case GT:
	case AND:
	case OR:
		return 1;
	default:
		return 0;
	}
}

const char* num_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"NUM\",\"value\":\"",
							root->data.number.value);
	return str_concat(s, "\"}");
}
const char* id_dump(const ASTNode* root){
	char *s =
		str_concat("{\"type\":\"ID\",\"value\":\"", root->data.ident.value);
	return str_concat(s, "\"}");
}
const char* string_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"STRING\",\"value\":\"",
							root->data.string.value);
	return str_concat(s, "\"}");
}
const char* idx_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"IDX\",\"container\":",
							ast_dump(root->data.index.container));
	s = str_concat(s, ",\"index\":");
	s = str_concat(s, ast_dump(root->data.index.index));
	return str_concat(s, "}");
}
const char* call_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"CALL\",\"calle\":",
							ast_dump(root->data.call.calle));
	s = str_concat(s, ",\"args\":[");
	for (unsigned int i = 0; i < root->data.call.args_cnt; i++) {
		if (i > 0)
			s = str_concat(s, ",");
		s = str_concat(s, ast_dump(root->data.call.args[i]));
	}
	return str_concat(s, "]}");
}
const char* binop_dump(const ASTNode* root){
	if (!is_binop_type(root->type))
		return "null";
	const char *type = ast_type_name(root->type);
	const char *lhs = ast_dump(root->data.binop.lhs);
	const char *rhs = ast_dump(root->data.binop.rhs);
	char *s = str_concat("{\"type\":\"", type);
	s = str_concat(s, "\",\"lhs\":");
	s = str_concat(s, lhs);
	s = str_concat(s, ",\"rhs\":");
	s = str_concat(s, rhs);
	return str_concat(s, "}");
}

typedef const char* (*dumper)(const ASTNode* root);

const char *ast_dump(const ASTNode *root) {
	static dumper dumpers[] = {
		[ADD] = binop_dump, [SUB] = binop_dump,
		[MUL] = binop_dump, [DIV] = binop_dump,
		[EXP] = binop_dump, [EQ]  = binop_dump,
		[NEQ] = binop_dump, [LEQ] = binop_dump,
		[GEQ] = binop_dump, [LT]  = binop_dump,
		[GT]  = binop_dump, [AND] = binop_dump,
		[OR]  = binop_dump,
		[NUM] = num_dump,
		[ID]  = id_dump,
		[STRING] = string_dump,
		[IDX] = idx_dump,
		[CALL] = call_dump,
	};
	
	if (root == 0)
		return "null";

	return dumpers[root->type](root);
}
const char *json_read_string(CharStream *stream) {
	skipWhitespace(stream);
	if (cs_peek(stream) != '\"') {
		return 0;
	}
	cs_get(stream);
	char *value = malloc(MAX_STRING_LENGTH + 1);
	unsigned int size = 0;
	while (!cs_eof(stream) && (cs_peek(stream) != '\"')) {
		value[size] = cs_get(stream);
		size += 1;
		if (size > MAX_STRING_LENGTH) {
			return 0;
		}
	}
	cs_get(stream);
	value[size] = '\0';
	return value;
}

int try_consume_with_ws(CharStream *stream, const char *pattern) {
	unsigned int saved_cursor = get_cursor(stream);
	int size = 0;
	while (!cs_eof(stream) && (pattern[size] != '\0')) {
		skipWhitespace(stream);
		if (cs_peek(stream) != pattern[size]) {
			break;
		}
		cs_get(stream);
		size += 1;
	}
	if (pattern[size] != '\0') {
		set_cursor(stream, saved_cursor);
		return 0;
	}
	return 1;
}

ASTNodeType json_parse_type(const char *s) {
	for (unsigned int i = 0; i < TYPE_NAMES_COUNT; i++) {
		if (type_names[i] && str_eq(s, type_names[i]))
			return (ASTNodeType)i;
	}
	return (ASTNodeType)-1;
}

ASTNode *json_load_block(CharStream *stream);

int binop_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"lhs\":"))
		return 0;
	ASTNode *lhs = json_load_block(stream);
	if (!lhs)
		return 0;
	if (!try_consume_with_ws(stream, ",\"rhs\":"))
		return 0;
	ASTNode *rhs = json_load_block(stream);
	if (!rhs)
		return 0;
	result->type = type;
	result->data.binop.lhs = lhs;
	result->data.binop.rhs = rhs;
	return 1;
}

int num_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"value\":"))
		return 0;
	const char *val = json_read_string(stream);
	result->type = type;
	result->data.number.value = val;
	return 1;
}

int id_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"value\":"))
		return 0;
	const char *val = json_read_string(stream);
	result->type = type;
	result->data.ident.value = val;
	return 1;
}

int string_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"value\":"))
		return 0;
	const char *val = json_read_string(stream);
	result->type = type;
	result->data.string.value = val;
	return 1;
}

int idx_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"container\":"))
		return 0;
	ASTNode *container = json_load_block(stream);
	if (!container)
		return 0;
	if (!try_consume_with_ws(stream, ",\"index\":"))
		return 0;
	ASTNode *index = json_load_block(stream);
	if (!index)
		return 0;
	result->type = type;
	result->data.index.container = container;
	result->data.index.index = index;
	return 1;
}

int json_load_list(CharStream *stream, ASTNode ***out, unsigned int *out_cnt,
				   int max_items) {
	skipWhitespace(stream);
	if (cs_eof(stream) || cs_peek(stream) != '[')
		return 0;
	cs_get(stream);
	ASTNode **items = malloc(sizeof(ASTNode *) * max_items);
	unsigned int cnt = 0;
	skipWhitespace(stream);
	if (!cs_eof(stream) && cs_peek(stream) != ']') {
		items[cnt] = json_load_block(stream);
		if (!items[cnt])
			return 0;
		cnt++;
		while (!cs_eof(stream)) {
			skipWhitespace(stream);
			if (cs_peek(stream) == ']')
				break;
			if (cs_peek(stream) != ',')
				return 0;
			cs_get(stream);
			items[cnt] = json_load_block(stream);
			if (!items[cnt])
				return 0;
			cnt++;
		}
	}
	if (cs_eof(stream) || cs_peek(stream) != ']')
		return 0;
	cs_get(stream);
	*out = items;
	*out_cnt = cnt;
	return 1;
}

int call_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"calle\":"))
		return 0;
	ASTNode *calle = json_load_block(stream);
	if (!calle)
		return 0;
	if (!try_consume_with_ws(stream, ",\"args\":"))
		return 0;
	ASTNode **args;
	unsigned int args_cnt;
	if (!json_load_list(stream, &args, &args_cnt, MAX_CALL_ARGS))
		return 0;
	result->type = type;
	result->data.call.calle = calle;
	result->data.call.args = args;
	result->data.call.args_cnt = args_cnt;
	return 1;
}

typedef int (*loader)(CharStream *stream, ASTNode *result, ASTNodeType type);

static loader loaders[] = {
	[ADD] = binop_load, [SUB] = binop_load,
	[MUL] = binop_load, [DIV] = binop_load,
	[EXP] = binop_load, [EQ]  = binop_load,
	[NEQ] = binop_load, [LEQ] = binop_load,
	[GEQ] = binop_load, [LT]  = binop_load,
	[GT]  = binop_load, [AND] = binop_load,
	[OR]  = binop_load,
	[NUM] = num_load,
	[ID]  = id_load,
	[STRING] = string_load,
	[IDX] = idx_load,
	[CALL] = call_load,
};

ASTNode *json_load_block(CharStream *stream) {
	skipWhitespace(stream);
	if (cs_peek(stream) != '{')
		return 0;
	cs_get(stream);

	if (!try_consume_with_ws(stream, "\"type\":"))
		return 0;
	const char *type_str = json_read_string(stream);
	if (!type_str)
		return 0;

	ASTNodeType node_type = json_parse_type(type_str);
	if (node_type == (ASTNodeType)-1)
		return 0;

	ASTNode *result = malloc(sizeof(*result));
	if (!loaders[node_type] || !loaders[node_type](stream, result, node_type))
		return 0;

	skipWhitespace(stream);
	if (!cs_eof(stream) && cs_peek(stream) == '}')
		cs_get(stream);
	return result;
}

ASTNode *ast_load(const char *json) {
	CharStream stream = {json, 0, strlen(json)};
	return json_load_block(&stream);
}