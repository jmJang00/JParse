#include "pch.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <JCore/Warnings.h>
#include <JParse/GrammarParser.h>

GrammarParser::GrammarParser(GrammarRegistry* symbolTable)
{
	_registry = symbolTable;
}

void GrammarParser::SetError(GRAMMAR_ERROR code)
{
	if (!errflag)
	{
		errflag = code;
	}
}

const char* GrammarParser::ParseSpace(const char* str, int* n)
{
	while (*n > 0 && (*str == ' ' || *str == '\t'))
	{
		*n -= 1;
		str++;
	}

	return str;
}

const char* GrammarParser::ParseArrow(const char* str, int* n)
{
	if (*n < 2)
	{
		SetError(ARROW);
		return str;
	}

	if (str[0] == '-' && str[1] == '>')
	{
		*n -= 2;
		str += 2;
	}
	else
	{
		SetError(ARROW);
	}

	return str;
}


const char* GrammarParser::ParseLHS(const char* str, int* n)
{
	const char* tmpStr = str;
	int tmpN = *n;
	const GrammarSymbol* symbol = nullptr;
	tmpStr = ParseSymbol(tmpStr, &tmpN, &symbol);
	if (errflag)
	{
		SetError(LHS);
		return str;
	}

	rule.push_back(symbol);

	str = tmpStr;
	*n = tmpN;

	return str;
}

const char* GrammarParser::ParseSymbol(const char* str, int* n, const GrammarSymbol** symbol)
{
	if (*n < 3) 
	{
		SetError(SYMBOL);
		return str;
	}

	char buffer[MAX_PARSE_STRING_LEN];
	char delimiter = '\0';
	switch (*str)
	{
	case '<':
		delimiter = '>';
		break;
	case '(':
		delimiter = ')';
		break;
	case '[':
		delimiter = ']';
		break;
	case '{':
		delimiter = '}';
		break;
	default:
		SetError(SYMBOL);
		return str;
	}

	const char* tmpStr = str + 1;
	int tmpN = *n - 1;
	int size = 0;

	while (tmpN > 0 && *tmpStr != delimiter)
	{
		if (size >= MAX_PARSE_STRING_LEN - 1)
		{
			SetError(SYMBOL);
			return str;
		}

		tmpN--;
		buffer[size++] = *tmpStr++;
	}

	if (tmpN == 0 || *tmpStr != delimiter)
	{
		SetError(SYMBOL);
		return str;
	}

	buffer[size] = '\0';
	*symbol = _registry->Find(buffer);
	if (*symbol == nullptr)
	{
		CRASH(true);
	}

	str = tmpStr + 1;
	*n = tmpN - 1;

	return str;
}

const char* GrammarParser::ParseRHS(const char* str, int* n)
{
	const char* tmpStr = str;
	int tmpN = *n;
	const GrammarSymbol* symbol = nullptr;
	tmpStr = ParseSymbol(tmpStr, &tmpN, &symbol);
	if (errflag)
		return str;

	rule.push_back(symbol);

	tmpStr = ParseSpace(tmpStr, &tmpN);
	while (tmpN > 0)
	{
		tmpStr = ParseSymbol(tmpStr, &tmpN, &symbol);
		if (errflag)
		{
			errflag = NONE;
			break;
		}

		rule.push_back(symbol);

		tmpStr = ParseSpace(tmpStr, &tmpN);
	}

	str = tmpStr;
	*n = tmpN;

	return str;
}

bool GrammarParser::Parse(const char* str, int* n)
{
	str = ParseSpace(str, n);

	str = ParseLHS(str, n);
	if (errflag)
		return false;

	str = ParseSpace(str, n);

	str = ParseArrow(str, n);
	if (errflag)
		return false;

	str = ParseSpace(str, n);

	str = ParseRHS(str, n);
	if (errflag)
		return false;

	return true;
}
