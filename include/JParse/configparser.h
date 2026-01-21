#pragma once
#include <optional>
#include <string>
#include <map>
#include <JParse/BaseCompiler.h>
#include <JParse/Grammar.h>

typedef std::unordered_multimap<std::string, symbol_table> config_type;

class ConfigParser : public BaseCompiler
{
public:
	ConfigParser();

	~ConfigParser();

	void Init();

	bool Evaluate(LCRSTree* tree) override;

	const std::string* GetString(symbol_table& table, const std::string& name)
	{
		auto iter = table.find(name);
		if (iter != table.end())
		{
			const value_union& value = iter->second;
			if (value.index() == 2)
			{
				return &std::get<std::string>(value);
			}
		}

		return nullptr;
	}

	const double* GetFloat(symbol_table& table, const std::string& name)
	{
		auto iter = table.find(name);
		if (iter != table.end())
		{
			const value_union& value = iter->second;
			if (value.index() == 1)
			{
				return &std::get<double>(value);
			}
		}

		return nullptr;
	}

	const int* GetInt(symbol_table& table, const std::string& name)
	{
		auto iter = table.find(name);
		if (iter != table.end())
		{
			const value_union& value = iter->second;
			if (value.index() == 0)
			{
				return &std::get<int>(value);
			}
		}

		return nullptr;
	}

	config_type& GetTable()
	{
		return _table;
	}

private:
	config_type _table;
};

