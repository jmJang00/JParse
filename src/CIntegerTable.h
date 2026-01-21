#pragma once
#include <fstream>
#include <JCore/Warnings.h>

class CIntegerTable
{
public:
	CIntegerTable(int row, int col)
	{
		_table = new int[row * col];
		memset(_table, 0, sizeof(int) * row * col);
		_row = row;
		_col = col;
	}

	~CIntegerTable()
	{
		delete[] _table;
	}

	CIntegerTable(const CIntegerTable& other)
	{
		_row = other._row;
		_col = other._col;
		_table = new int[_row * _col];
		*this = other;
	}

	CIntegerTable(CIntegerTable&& other) noexcept
	{
		_row = other._row;
		_col = other._col;
		_table = other._table;
		other._table = nullptr;
		other._row = 0;
		other._col = 0;
	}

	bool Dump(const char* name)
	{
		std::ofstream file(name, std::ios::binary);
		if (file.is_open())
		{
			file.write((char*)&_row, sizeof(int));
			file.write((char*)&_col, sizeof(int));
			for (int i = 0; i < _row; ++i)
			{
				for (int j = 0; j < _col; ++j)
				{
					file.write((char*)(_table + _col * i + j), sizeof(int));
				}
			}

			return true;
		}

		return false;
	}

	static CIntegerTable* Load(const char* name)
	{
		std::ifstream file(name, std::ios::binary);
		if (file.is_open())
		{
			int row;
			int col;
			file.read((char*)&row, sizeof(row));
			file.read((char*)&col, sizeof(row));
			CIntegerTable* table = new CIntegerTable(row, col);

			for (int i = 0; i < table->_row; ++i)
			{
				for (int j = 0; j < table->_col; ++j)
				{
					file.read((char*)(table->_table + table->_col * i + j), sizeof(int));
				}
			}

			return table;
		}

		return nullptr;
	}

	CIntegerTable& operator=(const CIntegerTable& other)
	{
		if (other._row != _row || other._col != _col)
		{
			CRASH(true);
		}

		memcpy(_table, other._table, _row * _col * sizeof(int));
		return *this;
	}

	CIntegerTable& operator=(CIntegerTable&& other) noexcept
	{
		_row = other._row;
		_col = other._col;
		_table = other._table;
		other._table = nullptr;
		other._row = 0;
		other._col = 0;
	}

	bool operator==(const CIntegerTable& other)
	{
		return memcmp(_table, other._table, _row * _col * sizeof(int)) == 0;
	}

	int* operator[](int row)
	{
		return _table + row * _col;
	}

	int* operator+(int num)
	{
		return _table + num * _col;
	}

	int* operator-(int num)
	{
		return _table - num * _col;
	}

private:
	int _row;
	int _col;
	int* _table;
};
