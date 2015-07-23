#pragma once
#include "BigInt.h"
#include "Key.h"
#include "KeyPair.h"
#include "PrimeGenerator.h"
#include "RSA.h"

class CRSACrypt
{
private:
	KeyPair		m_keyPair;
	Key*		m_pPublicKey;

public:
	std::string GetPublicKeyString();
	void SetPublicKey(const std::string& str);
	std::string Encrypt(const std::string& message);
	std::string Decrypt(const std::string& message);

public:
	CRSACrypt(void);
	~CRSACrypt(void);
};

