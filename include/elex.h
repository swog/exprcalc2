#pragma once

typedef double (*elex_evalfn)(double left, double right);

enum elex_err : int {
	elex_success,
	elex_err_unk,	// Unknown operation
	elex_err_unclosed,
	elex_err_tmt,	// too many tokens
};

int ecalc(const char* str, double& res);
