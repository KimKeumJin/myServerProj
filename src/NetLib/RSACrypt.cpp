#include "StdAfx.h"
#include "RSACrypt.h"


CRSACrypt::CRSACrypt(void)
	:m_keyPair(RSA::GenerateKeyPair(8))
	,m_pPublicKey(NULL)
{
	SAFE_DELETE(m_pPublicKey);
}


CRSACrypt::~CRSACrypt(void)
{
}

std::string CRSACrypt::GetPublicKeyString()
{
	std::string rsa;

	std::string mod(m_keyPair.GetPublicKey().GetModulus().ToString());
	std::string exp(m_keyPair.GetPublicKey().GetExponent().ToString());

	char modLength[10] = {0,};
	_itoa_s(mod.length(), modLength, 10);
	rsa += modLength;
	rsa += mod;
	rsa += exp;

	return rsa;
}

void CRSACrypt::SetPublicKey( const std::string& str )
{
	int modlength = atoi(str.substr(0,1).c_str());
	int explength = str.length() - (modlength + 1);

	BigInt mod(str.substr(1, modlength));
	BigInt exp(str.substr(1 + modlength, explength));

	m_pPublicKey = new Key(mod, exp);

}

std::string CRSACrypt::Encrypt( const std::string& message )
{
	return RSA::Encrypt(message, *m_pPublicKey);
}

std::string CRSACrypt::Decrypt( const std::string& message )
{
	return RSA::Decrypt(message, m_keyPair.GetPrivateKey());
}

