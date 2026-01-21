#pragma once
#include <vector>
#include <JParse/Token.h>

class Grammar;
struct Rule;

class GrammarParser
{
public:
	inline static constexpr int MAX_PARSE_STRING_LEN = 128;

	GrammarParser(GrammarRegistry* symbolTable);

	enum GRAMMAR_ERROR
	{
		NONE = 0,
		LHS,
		RHS,
		SYMBOL,
		ARROW,
	};

	void SetError(GRAMMAR_ERROR code);

	const char* ParseSpace(const char* str, int* n);

	const char* ParseArrow(const char* str, int* n);

	const char* ParseLHS(const char* str, int* n);

	const char* ParseSymbol(const char* str, int* n, const GrammarSymbol** symbol);

	const char* ParseRHS(const char* str, int* n);

	bool Parse(const char* str, int* n);

	symbol_vector rule;

private:
	GRAMMAR_ERROR errflag = NONE;
	GrammarRegistry* _registry = nullptr;
};
