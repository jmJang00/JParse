#include "pch.h"
#include <stdio.h>
#include "Lookahead.h"
#include "First.h"
#include "Follow.h"

CLookahead::CLookahead(Grammar* grammar, CIntegerTable& first, CIntegerTable& follow)
	: _nonterminalCnt(grammar->GetRegistry()->GetNonterminalCnt())
	, _terminalCnt(grammar->GetRegistry()->GetTerminalCnt())
	, _grammar(grammar)
	, _first(first)
	, _follow(follow)
	, _recordLookahead(
		grammar->GetRuleCnt(), 
		grammar->GetRegistry()->GetTerminalCnt())
{

}

void CLookahead::ComputeLookaheads(symbol_vector& rule, int* lookahead)
{
	for (int i = 1; i < rule.size(); i++)
	{
		if (rule[i]->IsTerminal())
		{
			int terminalNum = rule[i]->GetTerminalNum();
			lookahead[terminalNum] |= 1;
		}
		else
		{
			int nonterminalNum = rule[i]->GetNonterminalNum();
			for (int j = 0; j < _terminalCnt; j++)
			{
				lookahead[j] |= _first[nonterminalNum][j];
			}
		}

		int ti = _grammar->GetRegistry()->Get((int)symbol_id::Epsilon)->GetTerminalNum();
		if (lookahead[ti])
		{
			lookahead[ti] = 0;
			if (i + 1 == rule.size())
			{
				int nonterminalNum = rule[0]->GetNonterminalNum();
				for (int j = 0; j < _terminalCnt; j++)
				{
					lookahead[j] |= _follow[nonterminalNum][j];
				}
			}
		}
		else
		{
			break;
		}
	}
}

void CLookahead::GetLookaheads()
{
	for (int i = 0; i < _grammar->GetRuleCnt(); i++)
	{
		symbol_vector& rule = _grammar->GetRule(i)->symbols;
		ComputeLookaheads(rule, _recordLookahead[i]);
	}
}

void CLookahead::PutLookaheads()
{
	printf("\n<lookahead>\n");
	for (int i = 0; i < _grammar->GetRuleCnt(); i++)
	{
		printf("LOOKAHEAD(");
		_grammar->Print(i);
		printf(") = ");

		for (int j = 0; j < _terminalCnt; j++)
		{
			if (_recordLookahead[i][j])
			{
				printf("<%s> ", _grammar->GetRegistry()->GetTerminal(j)->ToString());
			}
		}

		printf("\n");
	}
}
