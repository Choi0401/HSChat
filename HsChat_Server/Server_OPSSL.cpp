#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <functional>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ipc.h>



using namespace std;


void ShowCerts(SSL* ssl);

#define BUF_SIZE 1024      

int main()
{

	//SOCKET serv_sock, clnt_sock;
	int serv_sock, clnt_sock;	
	struct sockaddr_in serv_addr, clnt_addr;

	string msg;

	int strLen;
	int clntAdrSize;
	int ac = -1;
	char message[BUF_SIZE] = { 0, };
	char data[BUF_SIZE] = { 0, };

	/* SSL Context 및 관련 구조체를 선언한다. */
	SSL_CTX* ctx;
	SSL* ssl;
	X509* client_cert;
	char* str;
	char buf[4096] = {0,};
	const SSL_METHOD* method;


	/* SSL 관련 초기화 작업을 수행한다. */
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	//method = TLSv1_1_server_method();
	//method = SSLv23_server_method();		// 서버 메소드.
	
	method = SSLv23_server_method();		// 서버 메소드.
	ctx = SSL_CTX_new(method);                // 지정된 초기 값을 이용하여 SSL Context를 생성한다.

	if (!ctx)
	{
		perror("ctx");
		exit(1);
	}

	/* 사용하게 되는 인증서 파일을 설정한다. */
	if (SSL_CTX_use_certificate_file(ctx, "server.pem", SSL_FILETYPE_PEM) <= 0)
	{

		perror("ctx_use_Certificate");
		exit(1);
	}

	/* 암호화 통신을 위해서 이용하는 개인 키를 설정한다. */
	if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
	{

		perror("ctx_use_Privatekey");
		exit(1);
	}

	/* 개인 키가 사용 가능한 것인지 확인한다. */
	if (!SSL_CTX_check_private_key(ctx))
	{
		perror("Private key does not match the certificate public keyn");
		exit(1);
	}

	/* peer (클라이언트)의 인증서를 검증하도록 세트 */
	//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	


	/*Prepare TCP socket for receiving connections */
	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("serv_sock socket err");
		exit(1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(8282);

	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) // 주소할당
		perror("bind() error!");

	if (listen(serv_sock, 5) == -1)
		perror("listen() error!");

	clntAdrSize = sizeof(clnt_addr);

	printf("Waiting for client \n");
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, (socklen_t*)&clntAdrSize);

	if (clnt_sock == -1)
		perror("accept() error!");
	else {
		printf("Accept client \n\n");

		/* TCP connection is ready. Do server side SSL. */
		if ((ssl = SSL_new(ctx)) == NULL) // 설정된 Context를 이용하여 SSL 세션의 초기화 작업을 수행한다.
		{
			perror("SSL_new() error!");
		}
		SSL_set_fd(ssl, clnt_sock);
		SSL_CTX_set_verify(SSL_get_SSL_CTX(ssl), SSL_VERIFY_PEER, NULL);

		if (SSL_accept(ssl) == -1)    // SSL 세션을 통해 클라이언트의 접속을 대기한다.
		{
			perror("SSL_accept");
		}
		else {
			printf("SSL connection using %s \n", SSL_get_cipher(ssl));
			//ShowCerts(ssl);
		}
	}

	while (1)
	{		
		// SSL 세션을 통해서 클라이언트와 데이터를 송수신한다
		int ret_read = SSL_read(ssl, buf, sizeof(buf));
		cout << "Received Msg : " << buf << endl;
		int ret_write = SSL_write(ssl, buf, ret_read);
		cout << "Sent " <<  ret_write <<  " Bytes : " <<  endl;
		memset(buf, '\0', sizeof(buf));
		
	}




	close(serv_sock);
	close(clnt_sock);
	return 0;
}





void ShowCerts(SSL* ssl)
{
	X509* cert;
	char* line;

	/* 클라이언트의 인증서를 받는다. */
	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL)
	{
		printf("Clients certificates:\n");
		/* 인증서의 이름을 출력한다. */
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Subject: %s\n", line);
		OPENSSL_free(line);
		/* 인증서의 issuer를 출력한다. */
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		OPENSSL_free(line);
		X509_free(cert);
	}
	else
		printf("Client does not have certificate.\n");
}

