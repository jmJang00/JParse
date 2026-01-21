#pragma once
#include <string>

class SourceFile
{
public:
	SourceFile();
	~SourceFile();
	int ReadFile(const char* direcotry, const char* fileName);
	void Clear();
	void SkipUTF8BOM(FILE* fp);
	char* _buffer = nullptr;
	int _size = 0;
	std::string _filename;
};