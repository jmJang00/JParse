#include "pch.h"
#include <JParse/BaseCompiler.h>
#include <JParse/LCRSTree.h>
#include <JParse/Grammar.h>
#include <JParse/Token.h>
#include "First.h"
#include "Follow.h"
#include "Lookahead.h"
#include "LLParser.h"
#include "Scanner.h"
#include "TokenStream.h"

BaseCompiler::BaseCompiler(const char* name, bool verbose, bool caching)
{
	_grammar = new Grammar();
	_file = new SourceFile();
	_scanner = nullptr;
	_stream = nullptr;
	_parser = nullptr;
	_first = nullptr;
	_follow = nullptr;
	_lookahead = nullptr;
	_init = false;
	_name = name;
	_verbose = verbose;
	_caching = caching;
}

BaseCompiler::~BaseCompiler()
{
	Clear();
	delete _parser;
	_parser = nullptr;
	delete _lookahead;
	_lookahead = nullptr;
	delete _follow;
	_follow = nullptr;
	delete _first;
	_first = nullptr;
}

void BaseCompiler::Init()
{
	if (_verbose)
	{
		_grammar->PrintAll();
	}

	std::string backupName(_name);
	backupName += "_PT.bin";
	CIntegerTable* table;
	if (_caching)
	{
		table = CIntegerTable::Load(backupName.c_str());
		if (table != nullptr)
		{
			_parser = new LLParser(_grammar, table);
			_scanner = new Scanner(_grammar);
			_init = true;
			return;
		}
	}

	_first = new CFirst(_grammar, _verbose);

	if (!_first->GetFirsts())
	{
		printf("tokenize fail\n");
		return;
	}

	if (_verbose)
	{
		_first->PutFirsts();
	}

	_follow = new CFollow(_grammar, _first->GetTable(), _verbose);

	if (!_follow->GetFollows())
	{
		printf("tokenize fail\n");
		return;
	}

	if (_verbose)
	{
		_follow->PutFollows();
	}

	_lookahead = new CLookahead(_grammar, _first->GetTable(), _follow->GetTable());
	_lookahead->GetLookaheads();
	if (_verbose)
	{
		_lookahead->PutLookaheads();
	}

	_parser = new LLParser(_grammar);
	_parser->ConstructParsingTable(_lookahead->GetTable(), backupName.c_str());
	_scanner = new Scanner(_grammar);

	if (_verbose)
	{
		_parser->PutParsingTable();
	}

	_init = true;
}

void BaseCompiler::Clear()
{
	delete _stream;
	_stream = nullptr;
	if (_parser != nullptr)
	{
		_parser->Clear();
	}

	if (_scanner != nullptr)
	{
		_scanner->Clear();
	}

	if (_grammar != nullptr)
	{
		_grammar->GetSymbolTable()->clear();
	}
}

bool BaseCompiler::Run(SourceFile* file)
{
	if (!_init)
	{
		Init();
	}

	if (file == nullptr || file->_buffer == nullptr)
	{
		return false;
	}

	_file = file;

	_stream = new TokenStream();
	int tokenIdx = 0;

	TokenData* token;
	_scanner->Init(file, _stream);

	do
	{
		if (!_scanner->Tokenize())
		{
			printf("Tokenize Fail\n");
			return false;
		}

		token = _stream->GetLastToken();
		if (_verbose)
		{
			printf("token: %d <%s> line:%d column:%d\n", token->symbol->id, token->symbol->ToString(), token->lineNum, token->columnNum);
		}

	} while (token->symbol->id != (int)symbol_id::Eof);


	LCRSTree* tree = nullptr;
	if (!_parser->PredictiveParsing(_stream, &tree, _verbose))
	{
		printf("parse fail\n");
		return false;
	}

	if (_verbose)
	{
		tree->Print(tree->GetRoot(), 0);
	}

	tree->TraversePostOrder(Grammar::ApplyRule);

	if (_verbose)
	{
		tree->Print(tree->GetRoot(), 0);
	}

	if (!Evaluate(tree))
	{
		printf("evaluate fail\n");
		delete tree;
		return false;
	}

	delete tree;

	return true;
}

void BaseCompiler::SimplifyTree(LCRSTree* tree)
{
	tree->TraversePostOrder([](LCRSNode* node)
		{
			LCRSNode* parent = node->GetParent();
			if (parent == nullptr)
			{
				return true;
			}

			bool isLeaf = !node->HasChildren();
			bool isSameSymbol = node->GetData()->symbol == parent->GetData()->symbol;
			bool isNonTerm = node->GetData()->symbol->IsNontermnial();

			if (isLeaf && isNonTerm)
			{
				parent->DeleteChild(node, true);
			}
			else if (isSameSymbol && isNonTerm)
			{
				parent->DeleteChild(node, false);
			}
			return true;
		});
}

option_map ParseCommandArgv(int argc, char** argv)
{
	option_map options;
	std::string arg;
	int cnt = 1;

	while (cnt < argc)
	{
		std::string cmd = argv[cnt++];

		if (cmd.length() > 2 && cmd[0] == '-' && cmd[1] == '-')
		{
			cmd = cmd.substr(2, cmd.length() - 2);
			auto& option = options.insert({cmd, std::vector<std::string>()}).first->second;

			while (cnt < argc)
			{
				arg = argv[cnt];
				if (arg.length() <= 2 || cmd[0] != '-' || cmd[1] != '-')
				{
					break;
				}

				option.push_back(arg);
				++cnt;
			}
		}
	}

	return options;
}
