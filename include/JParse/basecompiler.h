#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <JParse/SourceFile.h>

class CFollow;
class CFirst;
class Scanner;
class CLookahead;
class LLParser;
class TokenStream;
class SourceFile;
class LCRSTree;
class Grammar;

class BaseCompiler
{
public:
	BaseCompiler(const char* name, bool verbose, bool caching);

	~BaseCompiler();

	virtual void Init();

	virtual void Clear();

	virtual bool Evaluate(LCRSTree* tree) = 0;

	bool Run(SourceFile* file);

	void SimplifyTree(LCRSTree* tree);

protected:
	const char* _name;
	bool _init;
	bool _verbose;
	bool _caching;
	SourceFile* _file;
	Scanner* _scanner;
	TokenStream* _stream;
	LLParser* _parser;
	Grammar* _grammar;
	CFirst* _first;
	CFollow* _follow;
	CLookahead* _lookahead;
};

typedef std::unordered_map<std::string, std::vector<std::string>> option_map;

extern option_map ParseCommandArgv(int argc, char** argv);