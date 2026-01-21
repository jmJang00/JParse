#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <JParse/SourceFile.h>

SourceFile::SourceFile()
{

}

SourceFile::~SourceFile()
{
	Clear();
}

int SourceFile::ReadFile(const char* direcotry, const char* fileName)
{
	if (_buffer != nullptr)
	{
		free(_buffer);
		_buffer = nullptr;
	}

	FILE* fp;
	errno_t err;
	char filepath[64] = { "" };
	strcat_s(filepath, sizeof(filepath), direcotry);
	strcat_s(filepath, sizeof(filepath), "/");
	strcat_s(filepath, sizeof(filepath), fileName);
	err = fopen_s(&fp, filepath, "rb");
	if (fp == nullptr)
	{
		return 0;
	}

	SkipUTF8BOM(fp);

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	if (fileSize == -1)
	{
		return 0;
	}

	char* buffer = (char*)malloc(fileSize + 1);
	if (buffer == nullptr)
	{
		fclose(fp);
		return 0;
	}

	rewind(fp);
	if (!fread(buffer, fileSize, 1, fp))
	{
		free(buffer);
		fclose(fp);
		return 0;
	}

	fclose(fp);
	buffer[fileSize] = '\0';
	_buffer = buffer;
	_size = fileSize;
	_filename = fileName;

	return fileSize;
}

void SourceFile::Clear()
{
	delete _buffer;
	_buffer = nullptr;
}

void SourceFile::SkipUTF8BOM(FILE* fp)
{
   unsigned char bom[3];
   size_t n = fread(bom, 1, 3, fp);

   // EF BB BF → UTF-8 BOM
   if (n == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
   {
      return; // 그냥 그대로 (이미 3바이트 읽혀서 포인터 이동됨)
   }
   else
   {
      // BOM이 아니면 파일 포인터를 처음으로 되돌림
      fseek(fp, 0, SEEK_SET);
   }
}
