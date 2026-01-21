#pragma once
#include <variant>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <JCore/Warnings.h>

#define COMP_STR_LENGTH 256

enum class symbol_id
{
	Null = 0, 
	String, Number, Float, Dquotes, Colon,
	Comma, Lbracket, Rbracket, Eof,
	Lbrace, Rbrace, Epsilon, Equal, Semicolon,
	Lparen, Rparen, Id, Unknown,
	Nonterminal = 200,
};

inline const char* ToString(symbol_id symbol)
{
	static const char* symbolStr[] =
	{
		"null",
		// terminal
		"string", "number", "float", "dquotes", "colon",
		"comma", "lbracket", "rbracket", "eof",
		"lbrace", "rbrace", "epsilon", "equal", "semicolon",
		"lparen", "rparen", "id", "unknown"
	};

	return symbolStr[(int)symbol];
}

enum class SymbolType
{
	None,
	Keyword,
	Nonterminal,
	Terminal,
};

class LCRSNode;
typedef std::variant<int, double, std::string> value_union;
typedef std::function<bool(LCRSNode*)> callback_type;
class Grammar;

struct GrammarSymbol
{
	int id = 0;
	SymbolType type = SymbolType::None;
	std::string name;

	GrammarSymbol()
	{

	}

	GrammarSymbol(int id_, SymbolType type_, std::string name_)
	{
		id = id_;
		type = type_;
		name = name_;
	}

	bool IsTerminal() const
	{
		return type == SymbolType::Terminal || type == SymbolType::Keyword;
	}

	bool IsNontermnial() const
	{
		return type == SymbolType::Nonterminal;
	}

	bool IsKeyword() const
	{
		return type == SymbolType::Keyword;
	}

	int GetNonterminalNum() const
	{
		return id - ((int)symbol_id::Nonterminal + 1);
	}

	int GetTerminalNum() const
	{
		return id - ((int)symbol_id::Null + 1);
	}

	bool HasTokenValue() const
	{
		return (symbol_id)id == symbol_id::Float || 
			(symbol_id)id == symbol_id::Number || 
			(symbol_id)id == symbol_id::String || 
			(symbol_id)id == symbol_id::Id ||
			type == SymbolType::Keyword;
	}

	bool operator==(const GrammarSymbol& symbol) const
	{
		return symbol.id == id;
	}

	bool operator!=(const GrammarSymbol& symbol) const
	{
		return symbol.id != id;
	}

	const char* ToString() const
	{
		return name.c_str();
	}
};

typedef std::vector<const GrammarSymbol*> symbol_vector;

class GrammarRegistry
{
	int nonterminalId = (int)symbol_id::Nonterminal + 1;
	int terminalId = (int)symbol_id::Null + 1;
	std::unordered_map<std::string, int> lookup;
	std::vector<std::unique_ptr<GrammarSymbol>> symbols;

public:
	GrammarRegistry()
	{
		symbols.resize(1000);
	}

	bool AddSymbol(const std::string& name, SymbolType type)
	{
		auto iter = lookup.find(name);
		if (iter != lookup.end())
		{
			return false;
		}

		int id = 0;
		switch (type)
		{
		case SymbolType::Keyword:
			id = terminalId++;
			break;
		case SymbolType::Nonterminal:
			id = nonterminalId++;
			break;
		case SymbolType::Terminal:
			id = terminalId++;
			break;
		default:
			CRASH(true);
		}

		symbols[id] = std::make_unique<GrammarSymbol>(id, type, name);
		lookup[name] = id;
		return true;
	}

	const GrammarSymbol* Find(const std::string& name) const
	{
		auto it = lookup.find(name);
		if (it == lookup.end())
		{
			return nullptr;
		}
		else
		{
			return symbols[it->second].get();
		}
	}

	const GrammarSymbol* Get(int id)
	{
		if (id >= symbols.size())
		{
			return nullptr;
		}

		return symbols[id].get();
	}

	const GrammarSymbol* GetNonterminal(int index)
	{
		if (index >= nonterminalId - (int)symbol_id::Nonterminal - 1)
		{
			return nullptr;
		}

		return symbols[index + (int)symbol_id::Nonterminal + 1].get();
	}

	const GrammarSymbol* GetTerminal(int index)
	{
		if (index >= terminalId - (int)symbol_id::Null - 1)
		{
			return nullptr;
		}

		return symbols[index + (int)symbol_id::Null + 1].get();
	}

	int GetNonterminalCnt() const
	{
		return nonterminalId - (int)symbol_id::Nonterminal - 1;
	}

	int GetTerminalCnt() const
	{
		return terminalId - (int)symbol_id::Null - 1;
	}
};

extern std::map<std::string, int, std::less<std::string>> keywords;

struct TokenData
{
	int lineNum;
	int columnNum;
	const GrammarSymbol* symbol;
	value_union value;

	TokenData()
		: lineNum(1)
		, symbol(nullptr)
		, value(0)
		, columnNum(0)
	{
	}

	TokenData(const GrammarSymbol* symbol)
		: lineNum(1)
		, symbol(symbol)
		, value(0)
		, columnNum(0)
	{
	}

	~TokenData()
	{
	}
};

struct ASTData
{
	enum ValueType
	{
		Null,
		Int,
		Float,
		String,
		Token,
		BaseNode,
	};

	Grammar* grammar = nullptr;
	TokenData* token = nullptr;
	const GrammarSymbol* symbol = nullptr;
	int ruleIdx = -1;
	ValueType type = Null;
	void* ptr = nullptr;

	const std::string& GetSymbolName()
	{
		return symbol->name;
	}

	bool HasData()
	{
		switch (type)
		{
		case Null:
			return false;
		case Int:
		case Float:
		case String:
			return true;
		case Token:
			return false;
		default:
			return false;
		}
	}
};
