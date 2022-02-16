#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>

class COpenSSL
{
public:
	SSL_CTX* m_pCTX;
	SSL* m_pSSL;
	X509* m_pServercert;

	COpenSSL();
	void m_InitCTX();
	int m_CheckCertKey();
	void m_ShowCerts();

};

