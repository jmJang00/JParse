#pragma once
#include <vector>
#include <JParse/Token.h>

class TokenStream
{
public:
	TokenStream()
		: _index(0)
	{

	}

	~TokenStream()
	{
		for (int i = 0; i < _tokens.size(); i++)
		{
			delete _tokens[i];
		}
		_tokens.clear();
		_index = 0;
	}

	void AddToken(TokenData* token)
	{
		_tokens.push_back(token);
	}

	TokenData* GetCurrentToken()
	{
		return _tokens[_index];
	}

	TokenData* GetLastToken()
	{
		if (_tokens.size() > 0)
		{
			return _tokens.back();
		}

		return nullptr;
	}

	TokenData* Next()
	{
		if (_index + 1 < _tokens.size())
		{
			_index++;
			return _tokens[_index];
		}

		return nullptr;
	}

	int _index;
	std::vector<TokenData*> _tokens;
};
