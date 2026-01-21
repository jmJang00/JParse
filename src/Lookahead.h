#pragma once
#include <JParse/Token.h>
#include <JParse/Grammar.h>
#include "CIntegerTable.h"

class CLookahead
{
public:
	CLookahead(Grammar* grammar, CIntegerTable& first, CIntegerTable& follow);

	void ComputeLookaheads(symbol_vector& rule, int* lookahead);

	void GetLookaheads();

	void PutLookaheads();

	CIntegerTable& GetTable()
	{
		return _recordLookahead;
	}

	Grammar* _grammar;
	CIntegerTable& _first;
	CIntegerTable& _follow;
	int _nonterminalCnt;
	int _terminalCnt;
	CIntegerTable _recordLookahead;
};

