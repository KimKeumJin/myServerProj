#pragma once

#define Cryptable_length  4096

class CSeedCrypt
{
private:
	char				m_encodeTable[Cryptable_length];
	char				m_decodeTable[Cryptable_length];

public:
	bool SetTable(unsigned int seed);

public:
	CSeedCrypt(void);
	~CSeedCrypt(void);
};

