#include "pch.h"
#include <stdio.h>
#include <JParse/Token.h>
#include <JParse/Grammar.h>
#include "First.h"

CFirst::CFirst(Grammar* grammar, bool verbose)
	: _nonterminalCnt(grammar->GetRegistry()->GetNonterminalCnt())
	, _terminalCnt(grammar->GetRegistry()->GetTerminalCnt())
	, _verbose(verbose)
	, _grammar(grammar)
	, _recordFirst(
		grammar->GetRegistry()->GetNonterminalCnt(), 
		grammar->GetRegistry()->GetTerminalCnt())
{

}

void CFirst::ComputeFirst(symbol_vector& currCfg, int* currRecordFirst)
{
	int epsIdx = _grammar->GetRegistry()->Get((int)symbol_id::Epsilon)->GetTerminalNum();
	
	for (int i = 1; i < currCfg.size(); i++)
	{
		if (i == 1 || currRecordFirst[epsIdx])
		{
			currRecordFirst[epsIdx] = 0;
			if (currCfg[i]->IsTerminal())
			{
				int tIdx = currCfg[i]->GetTerminalNum();
				currRecordFirst[tIdx] |= 1;
			}
			else
			{
				int ntIdx = currCfg[i]->GetNonterminalNum();
				for (int tIdx = 0; tIdx < _terminalCnt; tIdx++)
				{
					currRecordFirst[tIdx] |= _recordFirst[ntIdx][tIdx];
				}
			}
		}
		else
		{
			break;
		}
	}
}

bool CFirst::GetFirsts()
{
	int i, j;
	const GrammarSymbol* currNonterminal;

	CIntegerTable oldRecordFirst(_recordFirst);

	int cnt = 0;
	while (cnt < 100)
	{
		oldRecordFirst = _recordFirst;

		for (i = 0; i < _nonterminalCnt; i++)
		{
			currNonterminal = _grammar->GetRegistry()->GetNonterminal(i);

			for (j = 0; j < _grammar->GetRuleCnt(); j++)
			{
				symbol_vector& currCfg = _grammar->GetRule(j)->symbols;
				if (currNonterminal == currCfg[0])
				{
					ComputeFirst(currCfg, _recordFirst[i]);
				}
			}
		}

		if (oldRecordFirst == _recordFirst)
		{
			break;
		}

		cnt++;
	}

	if (cnt == 100)
	{
		printf("error: GetFirsts() may have infinite loop\n");
		return false;
	}

	if (_verbose)
	{
		printf("GetFirsts() loop cnt:%d\n", cnt);
	}

	return true;
}

void CFirst::PutFirsts()
{
	int i, j;
	printf("\n<first>\n");
	for (i = 0; i < _nonterminalCnt; i++)
	{
		printf("FIRST(<%s>) = ", _grammar->GetRegistry()->GetNonterminal(i)->ToString());

		for (j = 0; j < _terminalCnt; j++)
		{
			if (_recordFirst[i][j])
			{
				printf("<%s> ", _grammar->GetRegistry()->GetTerminal(j)->ToString());
			}
		}
		printf("\n");
	}
}
