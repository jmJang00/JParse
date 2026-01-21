#pragma once
#include <unordered_map>
#include <functional>
#include <vector>
#include <JParse/Token.h>
#include <JParse/LCRSTree.h>
#include <JParse/GrammarParser.h>

struct Rule
{
	callback_type func;
	symbol_vector symbols;
};

typedef std::unordered_map<std::string, value_union> symbol_table;

class Grammar
{
public:
	inline static constexpr int CFGSIZE = 100;
	inline static constexpr int RULESIZE = 26;

	Grammar();

	void Print(int i);

	void PrintAll();

	void DefineSymbol(std::string str, SymbolType type);

	void AddRule(std::string str, callback_type func);

	GrammarRegistry* GetRegistry();

	Rule* GetRule(int idx);

	symbol_table* GetSymbolTable()
	{
		return &_symbolTable;
	}

	int GetRuleCnt();

	static bool Skip(LCRSNode* node);
	static bool DeleteNode(LCRSNode* node);
	static bool AdoptChild(LCRSNode* node);
	static bool CreateFloat(LCRSNode* node);
	static bool CreateNumber(LCRSNode* node);
	static bool CreateString(LCRSNode* node);
	static bool Register(LCRSNode* node);
	static bool ApplyRule(LCRSNode* node);

private:
	int _ruleCnt = 0;
	Rule _cfg[RULESIZE];
	symbol_table _symbolTable;
	GrammarRegistry _registry;
	GrammarParser _parser;
	const GrammarSymbol* _nullSymbol;
};
