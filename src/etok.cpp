#include <ctype.h>
#include <stdlib.h>
#include <cstdio>
#include "etok.h"

/*
	tokenize expression
	return `0` when a good token is retrieved.
*/
int etok(	const char* str, size_t size, size_t& pos, 
		char* tok, size_t tok_size, 
		etok_type& type
) {
	if (pos >= size) {
		return etok_err_eos;
	}

	/* Skip to non-blank. */
	while (str[pos] && isblank(str[pos])) {
		pos++;
	}

	/* End of string. */
	if (!str[pos]) {
		return etok_err_eos;
	}

	/* Punctuation are their own token. */
	if (ispunct(str[pos])) {
		/* Assert size. */
		if (tok_size < 2) {	
			return etok_err_oob;
		}

		tok[0] = str[pos++];
		tok[1] = '\0';
		type = etok_type_punct;

		return etok_err_none;
	}

	/* Skip to alphanumeric. */
	while (str[pos] && !isalnum(str[pos])) {
		pos++;
	}

	size_t len = 0;

	/* Iterate over alphanumerics */
	while (str[pos] && 
		(isalnum(str[pos]) || str[pos] == '.' || str[pos] == '_')) {
		/* Concatenate into token. */
		if (len < tok_size) {
			tok[len++] = str[pos++];
		}
		/* Out of bounds error. */
		else {
			return etok_err_oob;
		}
	}

	/* Out of bounds for null-terminator. */
	if (len >= tok_size) {
		return etok_err_oob;
	}

	tok[len++] = '\0';

	if (isalpha(tok[0])) {
		type = etok_type_alpha;
	}
	else {
		type = etok_type_num;
	}

	return etok_err_none;
}









