#pragma once
#include <vector>
#include <JParse/Token.h>

class Grammar;
class SourceFile;
class TokenStream;

class Scanner
{
public:
	Scanner(Grammar* grammar);

	~Scanner();

	void Init(SourceFile* file, TokenStream* stream);

	void Clear();

	bool Tokenize();

private:

	char GetCharacter();

	void UngetCharacter(char ch);

	void NextLine();

	TokenData* MakeToken(symbol_id id);

	TokenData* MakeToken(const GrammarSymbol* symbol);

	int GetIntNum(char ch);

	float GetFloatNum(char ch);

	int HexValue(char ch);

	void LexicalError(int n);

	bool IsLetter(char ch);

	bool IsLetterOrDigit(char ch);

	Grammar* _grammar;
	SourceFile* _file;
	TokenStream* _stream;
	int index;
	int lineNum;
	std::vector<int> columns;
};
