#include "pch.h"
#include <string>
#include <fstream>
#include <JParse/ConfigParser.h>
#include <JParse/SourceFile.h>
#include <JParse/LCRSTree.h>

ConfigParser::ConfigParser()
	: BaseCompiler("ConfigParser", false, false)
{
}

ConfigParser::~ConfigParser()
{
}

void ConfigParser::Init()
{
	_grammar->DefineSymbol("Group", SymbolType::Nonterminal);
	_grammar->DefineSymbol("Title", SymbolType::Nonterminal);
	_grammar->DefineSymbol("List", SymbolType::Nonterminal);
	_grammar->DefineSymbol("Entry", SymbolType::Nonterminal);
	_grammar->DefineSymbol("Middle", SymbolType::Nonterminal);
	_grammar->DefineSymbol("Value", SymbolType::Nonterminal);
	_grammar->DefineSymbol("Tag", SymbolType::Nonterminal);
	_grammar->DefineSymbol("thread", SymbolType::Keyword);

	_grammar->AddRule("[Start] -> [Group]", Grammar::Skip);
	_grammar->AddRule("[Group] -> [Title] <lbrace> [List] <rbrace> [Group]", Grammar::Skip);
	_grammar->AddRule("[Group] -> <epsilon>", Grammar::Skip);
	_grammar->AddRule("[Title] -> [Tag]", Grammar::AdoptChild);
	_grammar->AddRule("[Title] -> {thread}", Grammar::AdoptChild);
	_grammar->AddRule("[List] -> [Entry] [Middle]", Grammar::Skip);
	_grammar->AddRule("[Middle] -> [Entry] [Middle]", Grammar::Skip);
	_grammar->AddRule("[Middle] -> <epsilon>", Grammar::Skip);
	_grammar->AddRule("[Entry] -> [Tag] <equal> [Value]", Grammar::Register);
	_grammar->AddRule("[Tag] -> <id>", Grammar::CreateString);
	_grammar->AddRule("[Value] -> <float>", Grammar::CreateFloat);
	_grammar->AddRule("[Value] -> <number>", Grammar::CreateNumber);
	_grammar->AddRule("[Value] -> <string>", Grammar::CreateString);

	BaseCompiler::Init();
}

bool ConfigParser::Evaluate(LCRSTree* tree)
{
	config_type& groups = _table;
	std::string keyword;
	config_type::iterator iter;

	tree->TraversePreOrder([&](LCRSNode* node)
		{
			const std::string& name = node->GetData()->symbol->name;
			if (name == "Title")
			{
				ASTData* child1 = node->GetChild(0)->GetData();
				keyword = std::get<std::string>(child1->token->value);
				iter = groups.insert({ keyword, symbol_table() });
			}

			if (node->GetData()->symbol->name == "Entry")
			{
				ASTData* child1 = node->GetChild(0)->GetData();
				ASTData* child2 = node->GetChild(1)->GetData();
				if (child1->type == ASTData::String && child2->HasData())
				{
					iter->second.insert({std::get<std::string>(child1->token->value), child2->token->value});
				}
			}

			return true;
		});

	return true;
}
