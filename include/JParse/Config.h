#pragma once
#include <string>
#include <vector>
#include <JParse/Token.h>
#include <JParse/Grammar.h>
#include <JParse/ConfigParser.h>

enum class ConfigFieldType
{
	Int,
	Float,
	String,
	None,
};

struct FieldMeta
{
	std::string name;
	size_t offset = 0;
	ConfigFieldType type = ConfigFieldType::None;
	value_union defaultValue;
};

template <typename T>
class SectionMeta
{
public:
	SectionMeta(const std::string& name)
		: sectionName(name)
	{
	}

	void ChangeSectionName(const std::string& name)
	{
		sectionName = name;
	}

	template <typename U = T>
    SectionMeta& FieldString(const std::string& name, std::string U::* member, const std::string& defaultValue)
    {
        FieldMeta field;
        field.offset = (size_t)&(((T*)0)->*member);
        field.name = name;
        field.type = ConfigFieldType::String;
        field.defaultValue = defaultValue;
        
        fields[name] = field; 
        return *this;
    }

	template <typename U = T>
    SectionMeta& FieldFloat(const std::string& name, double U::* member, double defaultValue)
    {
        FieldMeta field;
        field.offset = (size_t)&(((T*)0)->*member);
        field.name = name;
        field.type = ConfigFieldType::Float;
        field.defaultValue = defaultValue;
        
        fields[name] = field; 
        return *this;
    }

	template <typename U = T>
    SectionMeta& FieldInt(const std::string& name, int U::* member, int defaultValue)
    {
        FieldMeta field;
        field.offset = (size_t)&(((T*)0)->*member);
        field.name = name;
        field.type = ConfigFieldType::Int;
        field.defaultValue = defaultValue;
        
        fields[name] = field; 
        return *this;
    }

	std::string sectionName;
	std::map<std::string, FieldMeta> fields;
};

class Config
{
public:
	void ConvertAndAssign(void* dst, const FieldMeta& meta, symbol_table& table)
	{
		const int* pInt = nullptr;
		const double* pFloat = nullptr;
		const std::string* pStr = nullptr;
		switch (meta.type)
		{
		case ConfigFieldType::Int:
		{
			if (pInt = parser.GetInt(table, meta.name))
			{
				printf("%d\n", *pInt);
				*(int*)((char*)dst + meta.offset) = *pInt;
			}
			else
			{
				printf("not found\n");
				*(int*)((char*)dst + meta.offset) = std::get<int>(meta.defaultValue);
			}
			break;
		}
		case ConfigFieldType::Float:
		{
			if (pFloat = parser.GetFloat(table, meta.name))
			{
				printf("%lf\n", *pFloat);
				*(double*)((char*)dst + meta.offset) = *pFloat;
			}
			else
			{
				printf("not found\n");
				*(double*)((char*)dst + meta.offset) = std::get<double>(meta.defaultValue);
			}
			break;
		}
		case ConfigFieldType::String:
		{
			if (pStr = parser.GetString(table, meta.name))
			{
				printf("%s\n", pStr->c_str());
				*(std::string*)((char*)dst + meta.offset) = *pStr;
			}
			else
			{
				printf("not found\n");
				*(std::string*)((char*)dst + meta.offset) = std::get<std::string>(meta.defaultValue);
			}
			break;
		}
		}
	}

	template <typename T>
	static SectionMeta<T> DefineSection(const std::string name)
	{
		return SectionMeta<T>(name);
	}

	bool Read(const char* name)
	{
		if (file.ReadFile("Config", name) == 0)
		{
			printf("Can't open %s\n", name);
			return false;
		}

		if (!parser.Run(&file))
		{
			printf("%s format disagreement\n", name);
			return false;
		}

		return true;
	}

	template <typename T>
	std::vector<T> Load(SectionMeta<T>& section)
	{
		auto range = parser.GetTable().equal_range(section.sectionName);

		printf("Load %s...\n", section.sectionName.c_str());
		std::vector<T> outConfigs;
		for (auto it = range.first; it != range.second; ++it)
		{
			outConfigs.push_back(T{});
			T& config = outConfigs.back();

			symbol_table& table = it->second;

			for (auto secIt = section.fields.begin(); secIt != section.fields.end(); secIt++)
			{
				const std::string name = secIt->first;
				FieldMeta& meta = secIt->second;
				printf("%s - %s: ", section.sectionName.c_str(), name.c_str());
				ConvertAndAssign(&config, meta, table);
			}
		}

		if (outConfigs.size() == 0)
		{
			printf("Fail\n");
		}
		else
		{
			printf("Complete\n");
		}

		return outConfigs;
	}

	SourceFile file;
	ConfigParser parser;
};

