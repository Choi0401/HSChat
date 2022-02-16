#include "pch.h"
#include "COpenSSL.h"

COpenSSL::COpenSSL()
{
    m_pCTX = NULL;
    m_pSSL = NULL;
    m_pServercert = NULL;
}

void COpenSSL::m_InitCTX()
{
    const SSL_METHOD* method;

    SSL_library_init();
    OpenSSL_add_all_algorithms();           /* Load cryptos, et.al. */
    SSL_load_error_strings();               /* Bring in and register error messages */

    method = SSLv23_client_method();        /* Create new client-method instance */
    m_pCTX = SSL_CTX_new(method);            /* Create new context */

    if (m_pCTX == NULL)
    {
        AfxMessageBox(_T("SSL_CTX_new() error"));
    }
}

int COpenSSL::m_CheckCertKey()
{
    if (SSL_CTX_use_certificate_file(m_pCTX, "client.pem", SSL_FILETYPE_PEM) <= 0)
    {
        //cout << "SSL_CTX_use_certificate_file() error" << endl;
        return 0;
    }

    if (SSL_CTX_use_PrivateKey_file(m_pCTX, "client.key", SSL_FILETYPE_PEM) <= 0) {
        //cout << "SSL_CTX_use_PrivateKey_file() error" << endl;
        return 0;
    }

    /* 개인 키가 사용 가능한 것인지 확인한다. */
    if (!SSL_CTX_check_private_key(m_pCTX)) {
        //cout << "SSL_CTX_check_private_key() error" << endl;
        return 0;
    }
    return 1;
}