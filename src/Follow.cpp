#include "pch.h"
#include <stdio.h>
#include <JParse/Grammar.h>
#include "Follow.h"
#include "First.h"

CFollow::CFollow(Grammar* grammar, CIntegerTable& first, bool verbose)
	: _nonterminalCnt(grammar->GetRegistry()->GetNonterminalCnt())
	, _terminalCnt(grammar->GetRegistry()->GetTerminalCnt())
	, _verbose(verbose)
	, _grammar(grammar)
	, _first(first)
	, _recordFollow(
		grammar->GetRegistry()->GetNonterminalCnt(), 
		grammar->GetRegistry()->GetTerminalCnt())
{

}

void CFollow::ComputeFollow(const GrammarSymbol* currNonterminal, int* currRecordFollow)
{
	// 시작 심볼일 때 $ 표시
	if (currNonterminal->GetNonterminalNum() == 0)
	{
		int tIdx = _grammar->GetRegistry()->Get((int)symbol_id::Eof)->GetTerminalNum();
		currRecordFollow[tIdx] = 1;
	}

	// 룰 중에 내 논터미널을 포함하고 있는 룰을 찾기 위해 루프를 돈다
	for (int rIdx = 0; rIdx < _grammar->GetRuleCnt(); rIdx++)
	{
		symbol_vector currCfg = _grammar->GetRule(rIdx)->symbols;
		const GrammarSymbol* currCfgNonterminal = _grammar->GetRule(rIdx)->symbols[0];

		for (int i = 1; i < currCfg.size(); i++)
		{
			// 내 논터미널을 발견하면 뒤를 보고 팔로우를 구한다
			if (currCfg[i] != currNonterminal)
			{
				continue;
			}

			int j = i + 1;
			while (1)
			{
				if (j == currCfg.size())
				{
					// 의미없는 계산일 때는 패스한다
					if (currCfgNonterminal != currNonterminal)
					{
						for (int tIdx = 0; tIdx < _terminalCnt; tIdx++)
						{
							_recordFollow[currNonterminal->GetNonterminalNum()][tIdx] |=
								_recordFollow[currCfgNonterminal->GetNonterminalNum()][tIdx];
						}
					}
					break;
				}
				else if (currCfg[j]->IsTerminal())
				{
					currRecordFollow[currCfg[j]->GetTerminalNum()] = 1;
				}
				else
				{
					int nonterminalNum = currCfg[j]->GetNonterminalNum();
					for (int tIdx = 0; tIdx < _terminalCnt; tIdx++)
					{
						currRecordFollow[tIdx] |= _first[nonterminalNum][tIdx];
					}
				}

				// 만약 엡실론이 발견되면 다음 문자도 살펴봐야 한다
				int epsIdx = _grammar->GetRegistry()->Get((int)symbol_id::Epsilon)->GetTerminalNum();
				if (!currRecordFollow[epsIdx])
				{
					break;
				}

				++j;
			}
		}
	}
}

bool CFollow::GetFollows()
{
	const GrammarSymbol* currNonterminal;

	CIntegerTable oldRecordFollow(_recordFollow);

	int cnt = 0;
	while (cnt < 100)
	{
		oldRecordFollow = _recordFollow;

		for (int i = 0; i < _nonterminalCnt; i++)
		{
			currNonterminal = _grammar->GetRegistry()->GetNonterminal(i);

			ComputeFollow(currNonterminal, _recordFollow[i]);
		}

		if (oldRecordFollow == _recordFollow)
		{
			break;
		}

		cnt++;
	}

	if (cnt == 100)
	{
		printf("error: GetFollows() may have infinite loop\n");
		return false;
	}

	if (_verbose)
	{
		printf("GetFollow() loop cnt:%d\n", cnt);
	}

	return true;
}

void CFollow::PutFollows()
{
	int i, j;
	printf("\n<follow>\n");
	for (i = 0; i < _nonterminalCnt; i++)
	{
		printf("FOLLOW(<%s>) = ", _grammar->GetRegistry()->GetNonterminal(i)->ToString());

		for (j = 0; j < _terminalCnt; j++)
		{
			if (_recordFollow[i][j])
			{
				printf("<%s> ", _grammar->GetRegistry()->GetTerminal(j)->ToString());
			}
		}
		printf("\n");
	}
}
