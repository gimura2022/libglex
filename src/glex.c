#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <glog.h>

#include "glex.h"

#define TEXT_BUFFER_SIZE 1024 * 4

glex__allocator_t glex__malloc    = NULL;
glex__deallocator_t glex__free    = NULL;
struct glog__logger* glex__logger = NULL;

void glex__init(glex__allocator_t allocator, glex__deallocator_t deallocator, struct glog__logger* logger)
{
	if (!allocator || !deallocator)
		return;

	glex__malloc = allocator;
	glex__free   = deallocator;
	glex__logger = logger;
}

void glex__set_token_defs(struct glex__lexer* lexer, size_t count, struct glex__token_def* arr)
{
	if (!lexer || !count || !arr)
		return;

	lexer->token_defs       = arr;
	lexer->token_defs_count = count;
}

static bool compare_token(struct glex__lexer* lexer, struct glex__token* tok);

bool glex__parse_string(struct glex__lexer* lexer, const char* str)
{
	if (!lexer || !str)
		return false;

	glog__debug(glex__logger, "starting parsing string");

	struct glex__token* tok_ptr = glex__malloc(sizeof(struct glex__token));
	glex__add_tok(lexer, tok_ptr);

	const char* c = str;
	while (*c != '\0') {
		glog__chaosf(glex__logger, "found char: %c", *c);

		for (int i = 0; i < lexer->token_defs_count; i++) if (
				(lexer->token_defs[i].def_type == GLEX__TOKENDEF_TYPE__SEPARATOR ||
				lexer->token_defs[i].def_type == GLEX__TOKENDEF_TYPE__IGNORE_SEPARATOR) &&
				lexer->token_defs[i].sym == *c) {
			glog__trace(glex__logger, "found separator");

			if (!compare_token(lexer, tok_ptr)) {
				glog__error(glex__logger, "syntax error");
				return false;
			}

			tok_ptr = glex__malloc(sizeof(struct glex__token));	
			tok_ptr->type = lexer->token_defs[i].type;
			glex__add_tok(lexer, tok_ptr);

			tok_ptr = glex__malloc(sizeof(struct glex__token));	
			glex__add_tok(lexer, tok_ptr);

			goto while_end;
		}

		char temp[2] = { *c, '\0' };
		strcat(tok_ptr->text, temp);

while_end:
		c++;
	}

	if (!compare_token(lexer, tok_ptr)) {
		glog__error(glex__logger, "syntax error");
		return false;
	}

	return true;
}

void glex__free_lexer(struct glex__lexer* lexer)
{
	if (!lexer)
		return;

	struct glex__token* tok = lexer->start;
	while (tok != NULL) {
		for (int i = 0; i < lexer->token_defs_count; i++) if (tok->type == lexer->token_defs[i].type
				&& lexer->token_defs[i].destructor != NULL) {
			lexer->token_defs[i].destructor(tok->data);
		}
		
		glex__free(tok->text);

		struct glex__token* next = tok->next;
		glex__free(tok);

		tok = next;
	}
}

struct glex__token* glex__get_tok(struct glex__lexer* lexer, int type)
{
	if (lexer->cur_tok == NULL)
		lexer->cur_tok = lexer->start;

	struct glex__token* tok = lexer->cur_tok;
	lexer->cur_tok = lexer->cur_tok->next;

	for (int i = 0; i < lexer->token_defs_count; i++) if (lexer->token_defs[i].type == type &&
			(lexer->token_defs[i].def_type == GLEX__TOKENDEF_TYPE__IGNORE_SEPARATOR ||
			 lexer->token_defs[i].def_type == GLEX__TOKENDEF_TYPE__IGNORE))
		return NULL;

	return tok->type == type ? tok : NULL;
}

void glex__add_tok(struct glex__lexer* lexer, struct glex__token* token)
{
	if (!lexer || !token)
		return;

	if (lexer->start == NULL || lexer->end == NULL) {
		lexer->start = token;
		lexer->end   = token;

		lexer->start->next = NULL;
		lexer->start->prev = NULL;

	} else {
		lexer->end->next = token;
		token->prev      = lexer->end;
		lexer->end       = token;
		lexer->end->next = NULL;
	}

	token->text = glex__malloc(TEXT_BUFFER_SIZE);
	memset(token->text, '\0', TEXT_BUFFER_SIZE);
}

void glex__read_int(void** data, const char* text)
{
	if (!data || !text)
		return;

	glog__chaosf(glex__logger, "parsing int %s", text);

	*data = glex__malloc(sizeof(int));
	**(int**) data = atoi(text);
}

bool glex__check_int(const char* str)
{
	const char* c = str;
	while (*c != '\0') {
		if (isalpha(*c))
			return false;

		c++;
	}

	return true;
}

void glex__free_int(void* data)
{
	if (!data)
		return;

	glex__free(data);
}

static bool compare_token(struct glex__lexer* lexer, struct glex__token* tok)
{
	bool found = false;
	for (int j = 0; j < lexer->token_defs_count; j++) if (
			(lexer->token_defs[j].def_type == GLEX__TOKENDEF_TYPE__TOKEN ||
			lexer->token_defs[j].def_type == GLEX__TOKENDEF_TYPE__IGNORE)) {
		if (!lexer->token_defs[j].text_chacker(tok->text)) continue;
		glog__chaos(glex__logger, "match found!");

		tok->type = lexer->token_defs[j].type;
		found     = true;

		lexer->token_defs[j].reader(&tok->data, tok->text);
		break;
	}

for_end:
	return found;
}
