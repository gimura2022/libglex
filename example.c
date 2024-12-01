#include <stdlib.h>
#include <string.h>

#include <glex.h>
#include <glog.h>

#define array_lenght(x) sizeof(x) / sizeof(x[0])

#define string_comparer(x, y) static bool x(const char* s) { return !strcmp(y, s); }

string_comparer(compare_http_ver, "HTTP/1.1")

int main(int argc, char* argv[])
{
	glog__init();

	struct glog__logger glex_logger    = {0};
	struct glog__logger example_logger = {0};
	glog__logger_from_prefix(&glex_logger, "glex");
	glog__logger_from_prefix(&example_logger, "example");

	glex_logger.min_log_level = 0;

	glex__init(malloc, free, &glex_logger);

	struct glex__lexer lexer = {0};
	struct glex__token_def token_defs[] = {
		(struct glex__token_def) {
			.def_type     = GLEX__TOKENDEF_TYPE__TOKEN,
			.type         = 0,
			.text_chacker = glex__check_int,

			.reader     = glex__read_int,
			.destructor = glex__free_int,
		},
		(struct glex__token_def) {
			.def_type     = GLEX__TOKENDEF_TYPE__TOKEN,
			.type         = 2,
			.text_chacker = compare_http_ver,

			.reader     = NULL,
			.destructor = NULL,
		},
		(struct glex__token_def) {
			.def_type   = GLEX__TOKENDEF_TYPE__IGNORE_SEPARATOR,
			.type       = 1,
			.sym        = ' ',

			.reader     = NULL,
			.destructor = NULL,
		},
	};
	glex__set_token_defs(&lexer, array_lenght(token_defs), token_defs);
	glex__parse_string(&lexer, "HTTP/1.1 100 200");

	struct glex__token* tok;
#	define except_token(x) if ((tok = glex__get_tok(&lexer)) == NULL || tok->type != x) \
		glog__dief(&example_logger, "unexepted token %i", tok == NULL ? -1 : tok->type);

	except_token(2);
	except_token(0);
	except_token(0);

#	undef except_token

	glex__free_lexer(&lexer);

	return 0;
}
