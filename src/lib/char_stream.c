#include "lib/char_stream.h"

void skipWhitespace(CharStream *stream) {
	while (cs_peek(stream) == ' ' || cs_peek(stream) == '\n') {
		cs_get(stream);
	}
}

unsigned int get_cursor(CharStream *stream) { return stream->cursor; }

void set_cursor(CharStream *stream, unsigned int cursor) {
	stream->cursor = cursor;
}

char cs_peek(CharStream *stream) { return stream->buffer[stream->cursor]; }

char cs_get(CharStream *stream) {
	char c = stream->buffer[stream->cursor];
	stream->cursor += 1;
	return c;
}

int cs_eof(CharStream *stream) { return stream->cursor >= stream->size; }
