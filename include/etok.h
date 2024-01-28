#ifndef _ETOK_H
#define _ETOK_H

/*
	Tokenize expression
	Return `NULL` when the end is reached.
*/
const char* etok(const char* str, char* tok, size_t tok_size);

#endif /* _ETOK_H */
