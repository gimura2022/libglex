#ifndef _glex_h
#define _glex_h

#include <stddef.h>
#include <stdbool.h>

#include <glog.h>

struct glex__token {
	int type;
	char* text;
	void* data;

	struct glex__token *next, *prev;
};

typedef void (*glex__token_data_reader_t)(void*, const char*);
typedef void (*glex__token_data_destructor_t)(void*);
typedef bool (*glex__token_text_checker_t)(const char*);

enum glex__token_def_type {
	GLEX__TOKENDEF_TYPE__TOKEN = 0,
	GLEX__TOKENDEF_TYPE__SEPARATOR,
	GLEX__TOKENDEF_TYPE__IGNORE,
	GLEX__TOKENDEF_TYPE__IGNORE_SEPARATOR,
};

struct glex__token_def {
	enum glex__token_def_type def_type;
	int type;

	union {
		char sym;
		glex__token_text_checker_t text_chacker;
	};

	glex__token_data_reader_t reader;
	glex__token_data_destructor_t destructor;
};

struct glex__lexer {
	size_t token_defs_count;
	struct glex__token_def* token_defs;

	struct glex__token *cur_tok, *start, *end;
};

typedef void* (*glex__allocator_t)(size_t);
typedef void (*glex__deallocator_t)(void*);

extern glex__allocator_t glex__malloc;
extern glex__deallocator_t glex__free;
extern struct glog__logger* glex__logger;

void glex__init(glex__allocator_t allocator, glex__deallocator_t deallocator, struct glog__logger* logger);
void glex__set_token_defs(struct glex__lexer* lexer, size_t count, struct glex__token_def* arr);

bool glex__parse_string(struct glex__lexer* lexer, const char* str);
void glex__free_lexer(struct glex__lexer* lexer);

struct glex__token* glex__get_tok(struct glex__lexer* lexer, int type);
void glex__add_tok(struct glex__lexer* lexer, struct glex__token* token);

void glex__read_int(void** data, const char* text);
bool glex__check_int(const char* str);
void glex__free_int(void* data);

#endif
