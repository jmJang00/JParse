#include "pch.h"
#include <JParse/Grammar.h>
#include <JParse/GrammarParser.h>
#include <stdio.h>

/*
1. A -> aAb
2. A -> bB
3. B -> aB
4. B -> b
*/

Grammar::Grammar()
	: _parser(&_registry)
{
	for (int i = 1; i < (int)symbol_id::Unknown; ++i)
	{
		_registry.AddSymbol(ToString((symbol_id)i), SymbolType::Terminal);
	}

	_registry.AddSymbol("Start", SymbolType::Nonterminal);

	//TODO: 널 심볼은 필요없어짐 나중에 제거
	_nullSymbol = _registry.Get((int)symbol_id::Null);

}

void Grammar::Print(int i)
{
	symbol_vector& currCFG = GetRule(i)->symbols;
	printf(" <%s> -> ", currCFG[0]->ToString());
	for (int i = 1; i < currCFG.size(); ++i)
	{
		printf("<%s> ", currCFG[i]->ToString());
	}
}

void Grammar::PrintAll()
{
	int i;
	bool bSameNonterminal = false;
	const GrammarSymbol* prevNonterminal = _nullSymbol;

	printf("\n<input cfg>\n");
	for (i = 0; i < _ruleCnt; i++)
	{
		if (prevNonterminal != _nullSymbol && prevNonterminal == GetRule(i)->symbols[0])
		{
			bSameNonterminal = true;
		}

		symbol_vector& currCFG = GetRule(i)->symbols;
		prevNonterminal = GetRule(i)->symbols[0];

		if (bSameNonterminal)
		{
			printf("| ");
			bSameNonterminal = false;
		}
		else
		{
			if (i == 0)
				printf("<%s> -> ", currCFG[0]->ToString());
			else
				printf("\n<%s> -> ", currCFG[0]->ToString());
		}

		for (int j = 1; j < currCFG.size(); ++j)
		{
			printf("<%s> ", currCFG[j]->ToString());
		}
	}
	printf("\n");
}

void Grammar::AddRule(std::string str, callback_type func)
{
	int n = (int)str.size();
	if (_parser.Parse(str.c_str(), &n))
	{
		int cnt = _ruleCnt++;
		_cfg[cnt].symbols = _parser.rule;
		_cfg[cnt].func = func;
		_parser.rule.clear();
	}
}

GrammarRegistry* Grammar::GetRegistry()
{
	return &_registry;
}

void Grammar::DefineSymbol(std::string str, SymbolType type)
{
	if (!_registry.AddSymbol(str, type))
	{
		printf("AddSymbol failed\n");
	}
}

Rule* Grammar::GetRule(int idx)
{
	return &_cfg[idx];
}

int Grammar::GetRuleCnt()
{
	return _ruleCnt;
}

bool Grammar::Skip(LCRSNode* node)
{
	return true;
}

bool Grammar::DeleteNode(LCRSNode* node)
{
	LCRSNode* parent = node->GetParent();
	parent->DeleteChild(node, false);
	return true;
}

bool Grammar::AdoptChild(LCRSNode* node)
{
	node->GetData()->type = ASTData::Token;
	node->GetData()->token = node->GetChild(0)->GetData()->token;
	return true;
}

bool Grammar::CreateFloat(LCRSNode* node)
{
	node->GetData()->type = ASTData::Float;
	node->GetData()->token = node->GetChild(0)->GetData()->token;
	return true;
}

bool Grammar::CreateNumber(LCRSNode* node)
{
	node->GetData()->type = ASTData::Int;
	node->GetData()->token = node->GetChild(0)->GetData()->token;
	return true;
}

bool Grammar::CreateString(LCRSNode* node)
{
	node->GetData()->type = ASTData::String;
	node->GetData()->token = node->GetChild(0)->GetData()->token;
	return true;
}

bool Grammar::Register(LCRSNode* node)
{
	ASTData* child1 = node->GetChild(0)->GetData();
	ASTData* child2 = node->GetChild(1)->GetData();
	if (child1->type == ASTData::String && child2->HasData())
	{
		node->GetData()->grammar->_symbolTable.insert({std::get<std::string>(child1->token->value), child2->token->value});
	}
	else
	{
		printf("[SetError] ApplyRule() register failed\n");
	}
	return true;
}

bool Grammar::ApplyRule(LCRSNode* node)
{
	if (node->GetData()->ruleIdx != -1)
	{
		Rule* g = node->GetData()->grammar->GetRule(node->GetData()->ruleIdx);
		g->func(node);
	}
	return true;
};
