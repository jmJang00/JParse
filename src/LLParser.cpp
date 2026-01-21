#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <JParse/LCRSTree.h>
#include <JParse/Grammar.h>
#include <JParse/Token.h>
#include "LLParser.h"
#include "Lookahead.h"

LLParser::LLParser(Grammar* grammar, CIntegerTable* parsingTable)
	: _nonterminalCnt(grammar->GetRegistry()->GetNonterminalCnt())
	, _terminalCnt(grammar->GetRegistry()->GetTerminalCnt())
	, _grammar(grammar)
{
	_epsilon = _grammar->GetRegistry()->Get((int)symbol_id::Epsilon);
	_eof = _grammar->GetRegistry()->Get((int)symbol_id::Eof);
	_start = _grammar->GetRegistry()->Find("Start");
	if (parsingTable != nullptr)
	{
		_parsingTable = parsingTable;
	}
	else
	{
		_parsingTable = new CIntegerTable(
			grammar->GetRuleCnt(),
			grammar->GetRegistry()->GetTerminalCnt());
	}
}

bool LLParser::PredictiveParsing(TokenStream* input, LCRSTree** outAstTree, bool verbose)
{
	_symbolStack.push({ _eof, nullptr});

	LCRSNode* startNode = LCRSTree::CreateNode(_grammar);
	startNode->GetData()->symbol = _start;
	LCRSTree* astTree = new LCRSTree(startNode);

	_symbolStack.push({ _start, startNode });

	CIntegerTable& parsingTable = *_parsingTable;

	TokenData* currToken = input->GetCurrentToken();
	int i;
	while (_symbolStack.size() > 1)
	{
		StackToken contextToken = _symbolStack.top();

		if (currToken->symbol == contextToken.symbol)
		{
			if (currToken->symbol->HasTokenValue())
			{
				auto node = contextToken.node;
				if (node != nullptr)
				{
					node->BorrowData(currToken);
				}
				else
				{
					printf("[error] shift node disappeared\n");
					delete astTree;
					Clear();
					return false;
				}
			}
			_symbolStack.pop();
			currToken = input->Next();
		}
		else if (contextToken.symbol->IsNontermnial())
		{
			int nonterminal = contextToken.symbol->GetNonterminalNum();
			int terminal = currToken->symbol->GetTerminalNum();

			if (parsingTable[nonterminal][terminal] == -1)
			{
				printf("[error] (%d:%d) Input:<%s> Expected:", currToken->lineNum, currToken->columnNum, currToken->symbol->ToString());
				for (int i = 0; i < _terminalCnt; ++i)
				{
					if (parsingTable[nonterminal][i] != -1)
					{
						printf("<%s> ", _grammar->GetRegistry()->GetTerminal(i)->ToString());
					}
				}
				printf("\n");
				delete astTree;
				Clear();
				return false;
			}

			int ruleIdx = parsingTable[nonterminal][terminal];
			symbol_vector& rule = _grammar->GetRule(ruleIdx)->symbols;
			if (verbose)
			{
				_grammar->Print(ruleIdx);
				printf("\n");
			}
			
			// start apply rule
			StackToken stackToken = _symbolStack.top();
			stackToken.node->GetData()->ruleIdx = ruleIdx;
			_symbolStack.pop();

			// input doesn't have epsilon token, so it can't be pushed in the stack
			auto parentNode = stackToken.node;
			LCRSNode* childNode = nullptr;
			int childCnt = 0;

			int len = (int)rule.size();
			for (i = len - 1; i >= 1; --i)
			{
				if (rule[i] == _epsilon)
					continue;

				LCRSNode* newNode = nullptr;
				bool isNt = rule[i]->IsNontermnial();

				// if new expanded grammar token is Nonterminal or has value to be saved,
				// it needs new node
				if (isNt || rule[i]->HasTokenValue())
				{
					newNode = LCRSTree::CreateNode(_grammar);

					newNode->GetData()->symbol = rule[i];

					parentNode->InsertFirst(newNode);

					childCnt++;
				}

				// TODO: 논터미널은 심볼테이블에서 찾아야 함
				// 아니면 처음부터 grammar가 심볼로 되어있으면 문제 없음
				_symbolStack.push({ rule[i], newNode});
			}
		}
		else
		{
			printf("[error] (%d:%d) Expected:<%s> -> Input:<%s>\n", currToken->lineNum, currToken->columnNum, contextToken.symbol->ToString(), currToken->symbol->ToString());
			delete astTree;
			Clear();
			return false;
		}
	}

	if (currToken->symbol == _eof && _symbolStack.top().symbol == _eof)
	{
		if (verbose)
		{
			printf("[accept]\n");
		}
		*outAstTree = astTree;
		Clear();
		return true;
	}
	else
	{
		printf("[error] stack is not empty\n");
		delete astTree;
		Clear();
		return false;
	}
}

void LLParser::Clear()
{
	while (_symbolStack.size() > 0)
	{
		_symbolStack.pop();
	}
}

void LLParser::ConstructParsingTable(CIntegerTable& lookahead, const char* name)
{
	CIntegerTable& parsingTable = *_parsingTable;

	for (int i = 0; i < _nonterminalCnt; i++)
	{
		for (int j = 0; j < _terminalCnt; j++)
		{
			parsingTable[i][j] = -1;
		}
	}

	bool conflict = false;

	for (int i = 0; i < _grammar->GetRuleCnt(); i++)
	{
		int nonterminal = _grammar->GetRule(i)->symbols[0]->GetNonterminalNum();
		for (int j = 0; j < _terminalCnt; j++)
		{
			if (lookahead[i][j])
			{
				if (parsingTable[nonterminal][j] != -1)
				{
					conflict = true;
				}

				parsingTable[nonterminal][j] = i;
			}
		}
	}

	if (conflict)
	{
		printf("parsing table conflict!\n");
		return;
	}

	parsingTable.Dump(name);
}

void LLParser::PutParsingTable()
{
	CIntegerTable& parsingTable = *_parsingTable;
	const char* fstr = "%10s";

	for (int ti = 0; ti <= _terminalCnt / 8; ti++)
	{
		printf("\n");
		printf(fstr, "Vn / Vt");
		printf(" | ");
		for (int i = ti * 8; i < std::min((ti + 1) * 8, _terminalCnt); i++)
		{
			printf(fstr, _grammar->GetRegistry()->GetTerminal(i)->ToString());
			printf(" | ");
		}
		printf("\n");

		printf("-------------");
		for (int i = ti * 8; i < std::min((ti + 1) * 8, _terminalCnt); i++)
		{
			printf("-------------");
		}
		printf("\n");

		for (int i = 0; i < _nonterminalCnt; i++)
		{
			printf(fstr, _grammar->GetRegistry()->GetNonterminal(i)->ToString());
			printf(" | ");

			for (int j = ti * 8; j < std::min((ti + 1) * 8, _terminalCnt); j++)
			{
				if (parsingTable[i][j] == -1)
					printf("          ");
				else
					printf("%10d", parsingTable[i][j]);

				printf(" | ");
			}

			printf("\n");
		}
	}
}
