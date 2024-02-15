#ifndef _ETOK_H
#define _ETOK_H

/* etok return values. */
enum etok_err : int {
	etok_err_none = 0, 	/* success = 0 */
	etok_err_eos,		/* end of string */
	etok_err_oob,		/* out of bounds */
};

enum etok_type : int {
	etok_type_null 	= 0,
	etok_type_punct = 1,
	etok_type_alpha = 2,
	etok_type_num 	= 4,
	// Not an actual type, flags
	etok_type_alnum = 6, 
};

/*
	This function tokenizes an expression.
return `0` when a good token is retrieved.
*/
int etok(	const char* str, size_t size, size_t& pos, 
		char* tok, size_t tok_size, 
		etok_type& type
);

#endif /* _ETOK_H */
