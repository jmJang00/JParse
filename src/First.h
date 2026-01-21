#pragma once
#include "CIntegerTable.h"

class Grammar;

class CFirst
{
public:
	CFirst(Grammar* grammar, bool verbose);

	void ComputeFirst(symbol_vector& currCfgRhs, int* currRecordFirst);

	bool GetFirsts();

	void PutFirsts();

	CIntegerTable& GetTable()
	{
		return _recordFirst;
	}

	Grammar* _grammar;
	int _nonterminalCnt;
	int _terminalCnt;
	CIntegerTable _recordFirst;
	bool _verbose;
};

