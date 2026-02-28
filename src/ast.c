#include "ast.h"
#include "lib/char_stream.h"
#include "lib/test.h"
#include "tokenizer.h"
#include "lib/memory.h"
#include "lib/string.h"


static const char *type_names[AST_NODE_TYPE_COUNT] = {
	[ADD] = "ADD", [MUL] = "MUL", [DIV] = "DIV", [SUB] = "SUB",
	[EXP] = "EXP", [EQ] = "EQ",   [NEQ] = "NEQ", [LEQ] = "LEQ",
	[GEQ] = "GEQ", [LT] = "LT",   [GT] = "GT",   [AND] = "AND",
	[OR] = "OR",   [NIL] = "NIL",  [NUM] = "NUM",  [ID] = "ID",   [STRING] = "STRING",
	[TABLE] = "TABLE",
	[IDX] = "IDX", [CALL] = "CALL",
	[NOT] = "NOT", [NEG] = "NEG",
	[IF_STMT] = "IF", [BLOCK] = "BLOCK",
	[WHILE_STMT] = "WHILE", [FOR_STMT] = "FOR",
	[CALL_STMT] = "CALL_STMT", [ASSIGN_STMT] = "ASSIGN_STMT",
	[FUNC_STMT] = "FUNC_STMT",
};

const char *ast_type_name(ASTNodeType type) {
	if (type < AST_NODE_TYPE_COUNT && type_names[type])
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
int is_unop_type(ASTNodeType type){
	switch(type){
		case NEG:
		case NOT:
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
const char* nil_dump(const ASTNode* root){
	(void)root;
	return "{\"type\":\"NIL\"}";
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
const char* table_dump(const ASTNode* root){
	char *s = "{\"type\":\"TABLE\",\"items\":[";
	for (unsigned int i = 0; i < root->data.table.items_cnt; i++) {
		if (i > 0)
			s = str_concat(s, ",");
		s = str_concat(s, ast_dump(root->data.table.items[i]));
	}
	return str_concat(s, "]}");
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
const char* unop_dump(const ASTNode* root){
	if (!is_unop_type(root->type)) return "null";
	const char *type = ast_type_name(root->type);
	const char *value = ast_dump(root->data.unop.value);
	char *s = str_concat("{\"type\":\"", type);
	s = str_concat(s, "\",\"value\":");
	s = str_concat(s, value);
	s = str_concat(s, "}");
	return s;
}

const char* block_dump(const ASTNode* root){
	char *s = "{\"type\":\"BLOCK\",\"body\":[";
	for (unsigned int i = 0; i < root->data.block.stmts_cnt; i++) {
		if (i > 0)
			s = str_concat(s, ",");
		s = str_concat(s, ast_dump(root->data.block.stmts[i]));
	}
	return str_concat(s, "]}");
}
const char* if_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"IF\",\"cond\":",
						ast_dump(root->data.if_node.cond));
	s = str_concat(s, ",\"then\":");
	s = str_concat(s, ast_dump(root->data.if_node.then_block));
	s = str_concat(s, ",\"else\":");
	s = str_concat(s, ast_dump(root->data.if_node.else_block));
	return str_concat(s, "}");
}
const char* while_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"WHILE\",\"cond\":",
						ast_dump(root->data.while_node.cond));
	s = str_concat(s, ",\"body\":");
	s = str_concat(s, ast_dump(root->data.while_node.body));
	return str_concat(s, "}");
}
const char* for_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"FOR\",\"name\":\"",
						root->data.for_node.name);
	s = str_concat(s, "\",\"in\":");
	s = str_concat(s, ast_dump(root->data.for_node.iterable));
	s = str_concat(s, ",\"body\":");
	s = str_concat(s, ast_dump(root->data.for_node.body));
	return str_concat(s, "}");
}
const char* call_stmt_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"CALL_STMT\",\"call\":",
						ast_dump(root->data.call_stmt.call));
	return str_concat(s, "}");
}
const char* assign_stmt_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"ASSIGN_STMT\",\"lhs\":",
						ast_dump(root->data.assign_stmt.lhs));
	s = str_concat(s, ",\"rhs\":");
	s = str_concat(s, ast_dump(root->data.assign_stmt.rhs));
	return str_concat(s, "}");
}
const char* func_stmt_dump(const ASTNode* root){
	char *s = str_concat("{\"type\":\"FUNC_STMT\",\"name\":\"",
						root->data.func_stmt.name);
	s = str_concat(s, "\",\"params\":[");
	for (unsigned int i = 0; i < root->data.func_stmt.params_cnt; i++) {
		if (i > 0)
			s = str_concat(s, ",");
		s = str_concat(s, "\"");
		s = str_concat(s, root->data.func_stmt.params[i]);
		s = str_concat(s, "\"");
	}
	s = str_concat(s, "],\"body\":");
	s = str_concat(s, ast_dump(root->data.func_stmt.body));
	return str_concat(s, "}");
}

typedef const char* (*dumper)(const ASTNode* root);

const char *ast_dump(const ASTNode *root) {
	static dumper dumpers[AST_NODE_TYPE_COUNT] = {
		[NEG] = unop_dump, [NOT] = unop_dump,
		[ADD] = binop_dump, [SUB] = binop_dump,
		[MUL] = binop_dump, [DIV] = binop_dump,
		[EXP] = binop_dump, [EQ]  = binop_dump,
		[NEQ] = binop_dump, [LEQ] = binop_dump,
		[GEQ] = binop_dump, [LT]  = binop_dump,
		[GT]  = binop_dump, [AND] = binop_dump,
		[OR]  = binop_dump,
		[NIL] = nil_dump,
		[NUM] = num_dump,
		[ID]  = id_dump,
		[STRING] = string_dump,
		[TABLE] = table_dump,
		[IDX] = idx_dump,
		[CALL] = call_dump,
		[BLOCK] = block_dump,
		[IF_STMT] = if_dump,
		[WHILE_STMT] = while_dump,
		[FOR_STMT] = for_dump,
		[CALL_STMT] = call_stmt_dump,
		[ASSIGN_STMT] = assign_stmt_dump,
		[FUNC_STMT] = func_stmt_dump,
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
	for (unsigned int i = 0; i < AST_NODE_TYPE_COUNT; i++) {
		if (type_names[i] && str_eq(s, type_names[i]))
			return (ASTNodeType)i;
	}
	return (ASTNodeType)-1;
}

ASTNode *json_load_block(CharStream *stream);
int json_load_list(CharStream *stream, ASTNode ***out, unsigned int *out_cnt,
		   int max_items);

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

int unop_load(CharStream *stream, ASTNode* result, ASTNodeType type){
	if (!try_consume_with_ws(stream, ",\"value\":")) {
		return 0;
	}
	ASTNode* value = json_load_block(stream);
	if(!value){
		return 0;
	}
	result->type = type;
	result->data.unop.value = value;
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

int nil_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	(void)stream;
	result->type = type;
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

int table_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"items\":"))
		return 0;
	ASTNode **items;
	unsigned int items_cnt;
	if (!json_load_list(stream, &items, &items_cnt, MAX_TABLE_ITEMS))
		return 0;
	result->type = type;
	result->data.table.items = items;
	result->data.table.items_cnt = items_cnt;
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

int block_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"body\":"))
		return 0;
	ASTNode **stmts;
	unsigned int stmts_cnt;
	if (!json_load_list(stream, &stmts, &stmts_cnt, MAX_BLOCK_STMTS))
		return 0;
	result->type = type;
	result->data.block.stmts = stmts;
	result->data.block.stmts_cnt = stmts_cnt;
	return 1;
}

int try_consume_null(CharStream *stream) {
	return try_consume_with_ws(stream, "null");
}

int if_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"cond\":"))
		return 0;
	ASTNode *cond = json_load_block(stream);
	if (!cond)
		return 0;
	if (!try_consume_with_ws(stream, ",\"then\":"))
		return 0;
	ASTNode *then_block = json_load_block(stream);
	if (!then_block)
		return 0;
	if (!try_consume_with_ws(stream, ",\"else\":"))
		return 0;
	ASTNode *else_block = 0;
	skipWhitespace(stream);
	if (!cs_eof(stream) && cs_peek(stream) != 'n')
		else_block = json_load_block(stream);
	else
		try_consume_null(stream);
	result->type = type;
	result->data.if_node.cond = cond;
	result->data.if_node.then_block = then_block;
	result->data.if_node.else_block = else_block;
	return 1;
}

int while_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"cond\":"))
		return 0;
	ASTNode *cond = json_load_block(stream);
	if (!cond)
		return 0;
	if (!try_consume_with_ws(stream, ",\"body\":"))
		return 0;
	ASTNode *body = json_load_block(stream);
	if (!body)
		return 0;
	result->type = type;
	result->data.while_node.cond = cond;
	result->data.while_node.body = body;
	return 1;
}

int for_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"name\":"))
		return 0;
	const char *name = json_read_string(stream);
	if (!name)
		return 0;
	if (!try_consume_with_ws(stream, ",\"in\":"))
		return 0;
	ASTNode *iterable = json_load_block(stream);
	if (!iterable)
		return 0;
	if (!try_consume_with_ws(stream, ",\"body\":"))
		return 0;
	ASTNode *body = json_load_block(stream);
	if (!body)
		return 0;
	result->type = type;
	result->data.for_node.name = name;
	result->data.for_node.iterable = iterable;
	result->data.for_node.body = body;
	return 1;
}

int call_stmt_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"call\":"))
		return 0;
	ASTNode *call = json_load_block(stream);
	if (!call)
		return 0;
	result->type = type;
	result->data.call_stmt.call = call;
	return 1;
}

int assign_stmt_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
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
	result->data.assign_stmt.lhs = lhs;
	result->data.assign_stmt.rhs = rhs;
	return 1;
}

int json_load_str_list(CharStream *stream, const char ***out, unsigned int *out_cnt,
				   int max_items) {
	skipWhitespace(stream);
	if (cs_eof(stream) || cs_peek(stream) != '[')
		return 0;
	cs_get(stream);
	const char **items = malloc(sizeof(const char *) * max_items);
	unsigned int cnt = 0;
	skipWhitespace(stream);
	if (!cs_eof(stream) && cs_peek(stream) != ']') {
		items[cnt] = json_read_string(stream);
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
			items[cnt] = json_read_string(stream);
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

int func_stmt_load(CharStream *stream, ASTNode *result, ASTNodeType type) {
	if (!try_consume_with_ws(stream, ",\"name\":"))
		return 0;
	const char *name = json_read_string(stream);
	if (!name)
		return 0;
	if (!try_consume_with_ws(stream, ",\"params\":"))
		return 0;
	const char **params;
	unsigned int params_cnt;
	if (!json_load_str_list(stream, &params, &params_cnt, MAX_FUNC_PARAMS))
		return 0;
	if (!try_consume_with_ws(stream, ",\"body\":"))
		return 0;
	ASTNode *body = json_load_block(stream);
	if (!body)
		return 0;
	result->type = type;
	result->data.func_stmt.name = name;
	result->data.func_stmt.params = params;
	result->data.func_stmt.params_cnt = params_cnt;
	result->data.func_stmt.body = body;
	return 1;
}

typedef int (*loader)(CharStream *stream, ASTNode *result, ASTNodeType type);

static loader loaders[AST_NODE_TYPE_COUNT] = {
	[NEG] = unop_load, [NOT] = unop_load,
	[ADD] = binop_load, [SUB] = binop_load,
	[MUL] = binop_load, [DIV] = binop_load,
	[EXP] = binop_load, [EQ]  = binop_load,
	[NEQ] = binop_load, [LEQ] = binop_load,
	[GEQ] = binop_load, [LT]  = binop_load,
	[GT]  = binop_load, [AND] = binop_load,
	[OR]  = binop_load,
	[NIL] = nil_load,
	[NUM] = num_load,
	[ID]  = id_load,
	[STRING] = string_load,
	[TABLE] = table_load,
	[IDX] = idx_load,
	[CALL] = call_load,
	[BLOCK] = block_load,
	[IF_STMT] = if_load,
	[WHILE_STMT] = while_load,
	[FOR_STMT] = for_load,
	[CALL_STMT] = call_stmt_load,
	[ASSIGN_STMT] = assign_stmt_load,
	[FUNC_STMT] = func_stmt_load,
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