#pragma once
#include <stack>
#include <JParse/Token.h>
#include <JParse/LCRSTree.h>
#include "TokenStream.h"
#include "CIntegerTable.h"

struct StackToken
{
	const GrammarSymbol* symbol;
	LCRSNode* node;
};

class LLParser
{
public:
	LLParser(Grammar* grammar, CIntegerTable* parsingTable = nullptr);

	bool PredictiveParsing(TokenStream* input, LCRSTree** outAstTree, bool verbose);

	void ConstructParsingTable(CIntegerTable& lookahead, const char* name);

	void PutParsingTable();

	void Clear();

	CIntegerTable* GetTable()
	{
		return _parsingTable;
	}

	std::stack<StackToken> _symbolStack;
	Grammar* _grammar;
	int _nonterminalCnt;
	int _terminalCnt;
	CIntegerTable* _parsingTable;
	const GrammarSymbol* _epsilon;
	const GrammarSymbol* _eof;
	const GrammarSymbol* _start;
};

