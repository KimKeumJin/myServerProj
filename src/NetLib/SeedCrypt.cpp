#include "StdAfx.h"
#include "SeedCrypt.h"
#include "RandomGen.h"

CSeedCrypt::CSeedCrypt(void)
{
	memset(m_encodeTable, 0, sizeof(m_encodeTable));
	memset(m_decodeTable, 0, sizeof(m_decodeTable));
}


CSeedCrypt::~CSeedCrypt(void)
{
}

bool CSeedCrypt::SetTable( unsigned int seed )
{
	CRandomGen random;
	random.Init(seed);

	for(int i = 0 ; i < Cryptable_length ; ++i)
	{
		do
		{
			m_encodeTable[i] = random.SimpleRandom() % 256;
		}while(m_encodeTable[i] == 0x00 || m_encodeTable[i] == 0xff);
	}

	for(int i = 0 ; i < Cryptable_length ; ++i)
	{
		do
		{
			m_decodeTable[i] = random.SimpleRandom() % 256;
		}while(m_decodeTable[i] == 0x00 || m_decodeTable[i] == 0xff);
	}

	return true;
}
