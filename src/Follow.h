#pragma once
#include <JParse/Token.h>
#include "CIntegerTable.h"

class CFollow
{
public:
	CFollow(Grammar* grammar, CIntegerTable& first, bool verbose);

	void ComputeFollow(const GrammarSymbol* currNonterminal, int* currRecordFollow);

	bool GetFollows();

	void PutFollows();

	CIntegerTable& GetTable()
	{
		return _recordFollow;
	}

	Grammar* _grammar;
	CIntegerTable& _first;
	int _nonterminalCnt;
	int _terminalCnt;
	CIntegerTable _recordFollow;
	bool _verbose;
};
