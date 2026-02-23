#ifndef CHAR_STREAM_H
#define CHAR_STREAM_H

typedef struct {
	const char *buffer;
	unsigned int cursor;
	unsigned int size;
} CharStream;

void skipWhitespace(CharStream *stream);
char cs_peek(CharStream *stream);
char cs_get(CharStream *stream);
int cs_eof(CharStream *stream);
unsigned int get_cursor(CharStream *stream);
void set_cursor(CharStream *stream, unsigned int cursor);

#endif
