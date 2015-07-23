#pragma once

#include <bitset>

template <typename T>
void OutputBitset(T data)
{
	std::bitset<sizeof(T) * 8> bit(data);

	for(int i = 0 ; i < sizeof(T) * 8 ; ++i)
	{
		if((i != 0) && (i % 8 == 0))
			std::cout << " ";

		std::cout << bit[i];
	}

	std::cout << endl;
}

void OutputBitset(char* pData, int length)
{
	for(int i = 0 ; i < length ; ++i)
	{
		std::bitset<8> bit(pData[i]);

		std::cout << bit << " ";
	}

	std::cout << std::endl;
}