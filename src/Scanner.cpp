#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdexcept>
#include <JCore/Warnings.h>
#include <JParse/Token.h>
#include <JParse/SourceFile.h>
#include <JParse/Grammar.h>
#include "Scanner.h"
#include "TokenStream.h"

Scanner::Scanner(Grammar* grammar)
{
	_grammar = grammar;
	_file = nullptr;
	_stream = nullptr;
	index = 0;
	lineNum = 1;
	columns.push_back(0);
}

Scanner::~Scanner()
{
	Clear();
}

void Scanner::Init(SourceFile* file, TokenStream* stream)
{
	_file = file;
	_stream = stream;
	index = 0;
	lineNum = 1;
	columns.push_back(0);
}

void Scanner::Clear()
{
	_file = nullptr;
	_stream = nullptr;
	index = 0;
	lineNum = 1;
	columns.clear();
}

char Scanner::GetCharacter()
{
	if (index > _file->_size)
	{
		throw std::runtime_error("getCharacter");
	}
	else
	{
		char ch = _file->_buffer[index++];
		columns[lineNum - 1]++;
		return ch;
	}
}

void Scanner::UngetCharacter(char ch)
{
	if (index > 0)
	{
		index--;
		if (columns[lineNum - 1] == 0)
		{
			lineNum--;
		}
		else
		{
			columns[lineNum - 1]--;
		}
	}
}

void Scanner::NextLine()
{
	lineNum++;
	if (columns.size() < lineNum)
	{
		columns.push_back(0);
	}
}

TokenData* Scanner::MakeToken(symbol_id id)
{
	TokenData* token = new TokenData();
	token->symbol = _grammar->GetRegistry()->Get((int)id);
	token->lineNum = lineNum;
	token->columnNum = columns[lineNum - 1];
	_stream->AddToken(token);
	CRASH(token == nullptr);
	return token;
}

TokenData* Scanner::MakeToken(const GrammarSymbol* symbol)
{
	TokenData* token = new TokenData();
	token->symbol = symbol;
	token->lineNum = lineNum;
	token->columnNum = columns[lineNum - 1];
	_stream->AddToken(token);
	return token;
}

int Scanner::GetIntNum(char ch)
{
	int num = 0;
	int value;

	if (ch != '0')
	{
		do
		{
			num = 10 * num + (int)(ch - '0');
			ch = GetCharacter();
		} while (isdigit(ch));
	}
	else
	{
		ch = GetCharacter();
		if ((ch >= '0') && (ch <= '7'))
		{
			do
			{
				num = 8 * num + (int)(ch - '0');
				ch = GetCharacter();
			} while ((ch >= '0') && (ch <= '7'));
		}
		else if ((ch == 'X') || (ch == 'x'))
		{
			while ((value = HexValue(ch = GetCharacter())) != -1)
			{
				num = 16 * num + value;
			}
		}
		else
		{
			num = 0;
		}
	}

	UngetCharacter(ch);
	return num;
}

float Scanner::GetFloatNum(char ch)
{
	int num = 0;
	int div = 1;
	do
	{
		ch = GetCharacter();
		if (!isdigit(ch))
		{
			break;
		}

		num = num * 10 + (ch - '0');
		div *= 10;
	} while (1);

	UngetCharacter(ch);
	return (float) num / div;
}

int Scanner::HexValue(char ch)
{
	switch (ch)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: 
		return -1;
	}
}

bool Scanner::IsLetter(char ch)
{
	if (isalpha(ch) || ch == '_')
		return 1;
	else
		return 0;
}

bool Scanner::IsLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') 
		return 1;
	else
		return 0;
}

void Scanner::LexicalError(int n)
{
	printf(" *** Lexcial Error ***\n");
	switch (n) 
	{
	case 1: 
		printf("an identifier length must be less than 12.\n");
		break;
	}
}

bool Scanner::Tokenize()
{
	int i;
	char ch;
	char str[COMP_STR_LENGTH];

	TokenData* token = nullptr;

	do 
	{
		while (isspace(ch = GetCharacter()))
		{
			if (ch == '\n')
			{
				NextLine();
			}
		}

		if (IsLetter(ch))
		{
			i = 0;
			do 
			{
				if (i < COMP_STR_LENGTH)
				{
					str[i++] = ch;
				}

				ch = GetCharacter();

			} while (IsLetterOrDigit(ch));

			if (i >= COMP_STR_LENGTH)
			{
				LexicalError(1);
				return false;
			}

			str[i] = '\0';

			const GrammarSymbol* symbol = _grammar->GetRegistry()->Find(str);
			if (symbol != nullptr && symbol->IsKeyword())
			{
				token = MakeToken(symbol);
			}
			else
			{
				token = MakeToken(symbol_id::Id);
				token->value = std::string(str);
			}

			UngetCharacter(ch);
		}
		else if (isdigit(ch))
		{
			int num = GetIntNum(ch);
			ch = GetCharacter();
			if (ch == '.')
			{
				double fnum = num + GetFloatNum(ch);
				token = MakeToken(symbol_id::Float);
				token->value = fnum;
			}
			else
			{
				token = MakeToken(symbol_id::Number);
				token->value = num;
				UngetCharacter(ch);
			}
		}
		else
		{
			switch (ch)
			{
			case '/':
				ch = GetCharacter();
				if (ch == '*')
				{
					do
					{
						while (ch != '*')
						{
							if (ch == '\n')
							{
								NextLine();
							}
							ch = GetCharacter();
						}
						ch = GetCharacter();

					} while (ch != '/');
				}
				else if (ch == '/')
				{
					while (GetCharacter() != '\n');
					UngetCharacter('\n');
				}
				break;
			case ':':
				token = MakeToken(symbol_id::Colon);
				break;
			case '"':
				i = 0;
				ch = GetCharacter();
				do 
				{
					if (i < COMP_STR_LENGTH)
					{
						str[i++] = ch;
					}

					ch = GetCharacter();

				} while (ch != '"');

				if (i >= COMP_STR_LENGTH)
				{
					LexicalError(1);
					return false;
				}

				str[i] = '\0';

				token = MakeToken(symbol_id::String);
				token->value = std::string(str);
				break;
			case ',':
				token = MakeToken(symbol_id::Comma);
				break;
			case '[':
				token = MakeToken(symbol_id::Lbracket);
				break;
			case ']':
				token = MakeToken(symbol_id::Rbracket);
				break;
			case '{':
				token = MakeToken(symbol_id::Lbrace);
				break;
			case '}':
				token = MakeToken(symbol_id::Rbrace);
				break;
			case '=':
				token = MakeToken(symbol_id::Equal);
				break;
			case ';':
				token = MakeToken(symbol_id::Semicolon);
				break;
			case '(':
				token = MakeToken(symbol_id::Lparen);
				break;
			case ')':
				token = MakeToken(symbol_id::Rparen);
				break;
			case '\0':
				token = MakeToken(symbol_id::Eof);
				return true;
			default:
				printf("Current character : %c\n", ch);
				LexicalError(4);
				return false;
			}
		}

	} while (token == nullptr);

	return true;
}
