#include <ctype.h>
#include <stdlib.h>
#include "etok.h"

/*
	Tokenize expression
	Return `NULL` when the end is reached.
*/
const char* etok(const char* str, char* tok, size_t tok_size) {
	/* Skip to non-blank. */
	while (*str && isblank(*str)) {
		str++;
	}

	/* End of string. */
	if (!*str) {
		return NULL;
	}

	/* Punctuation are their own token. */
	if (ispunct(*str)) {
		/* Assert size. */
		if (tok_size < 2) {	
			return NULL;
		}

		tok[0] = *str;
		tok[1] = '\0';

		return ++str;
	}

	/* Skip to alphanumeric. */
	while (*str && !isalnum(*str)) {
		str++;
	}

	size_t len = 0;

	/* Iterate over alphanumerics */
	while (*str && (isalnum(*str) || *str == '.' || *str == '_')) {
		/* Concatenate into token. */
		if (len < tok_size) {
			tok[len++] = *str;
		}
		/* Out of bounds error. */
		else {
			return NULL;
		}

		str++;
	}

	/* Out of bounds for null-terminator. */
	if (len >= tok_size) {
		return NULL;
	}

	tok[len++] = '\0';

	return str;
}









