#include <stdlib.h>

#include <glex.h>
#include <glog.h>

#define array_lenght(x) sizeof(x) / sizeof(x[0])

int main(int argc, char* argv[])
{
	glog__init();

	struct glog__logger glex_logger = {0};
	glog__logger_from_prefix(&glex_logger, "glex");

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
			.def_type   = GLEX__TOKENDEF_TYPE__IGNORE_SEPARATOR,
			.type       = 1,
			.sym        = ' ',

			.reader     = NULL,
			.destructor = NULL,
		},
	};
	glex__set_token_defs(&lexer, array_lenght(token_defs), token_defs);

	glex__parse_string(&lexer, "1113 2321 f");

	glex__free_lexer(&lexer);

	return 0;
}
