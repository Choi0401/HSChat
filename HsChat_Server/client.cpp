#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <vector>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define CHK_NULL(x) if((x) == NULL) exit(1);
#define CHK_ERR(err, s) if((err) == -1) { perror(s); exit(1); }
#define CHK_SSL(err) if((err) == -1) { ERR_print_errors_fp(stderr); exit(2); }

#define BUFSIZE 6000
#define MAXLISTEN 1000
#define MAX_FD_SOCKET 512
#define PORT 7777

int main() {
	int clnt_sock, err;
	struct sockaddr_in clnt_addr;
	socklen_t addr_len;
	char msg[BUFSIZE];
	char recvmsg[BUFSIZE];

	/* SSL 관련 정보를 관리할 구조체를 선언한다. */
    SSL_CTX   *ctx;
    SSL     *ssl;
    X509                    *server_cert;
    char                    *str;
    char                    buf[4096];
    const SSL_METHOD    *meth;
   
    /* 암호화 통신을 위한 초기화 작업을 수행한다. */
    SSL_load_error_strings();
    SSLeay_add_ssl_algorithms();
    meth = SSLv23_client_method();
    ctx = SSL_CTX_new(meth);
    CHK_NULL(ctx);
   
    /* 사용하게 되는 인증서 파일을 설정한다. – opt*/
    if(SSL_CTX_use_certificate_file(ctx, "client.pem", SSL_FILETYPE_PEM) <= 0) {    // 인증서를 파일로 부터 로딩할때 사용함.
        ERR_print_errors_fp(stderr);
        exit(3);
    }
   
    /* 암호화 통신을 위해서 이용하는 개인 키를 설정한다. – opt */
    if(SSL_CTX_use_PrivateKey_file(ctx, "client.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(4);
    }
   
    /* 개인 키가 사용 가능한 것인지 확인한다. – opt */
    if(!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate public key\n");
        exit(5);
    }

	clnt_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	memset(&clnt_addr, 0, sizeof(clnt_addr));

	addr_len = sizeof(clnt_addr);

	clnt_addr.sin_family = AF_INET;
	clnt_addr.sin_port = htons(atoi("7777"));
	clnt_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	connect(clnt_sock, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));

	/* Now we have TCP connection. Start SSL negotiation. */
    ssl = SSL_new(ctx);  // 세션을 위한 자원을 할당받는다.
    CHK_NULL(ssl);

	SSL_set_fd(ssl,clnt_sock);
    err = SSL_connect(ssl); // 기존의 connect() 함수 대신 사용하여 서버로 접속한다.
    //CHK_NULL(err);

	 /* Following two steps are optional and not required for data exchange to be successful. */

    /* Get the Cipher – opt */
    printf("SSL connection using %s\n", SSL_get_cipher(ssl));

    /* Get server’s certificate (note: beware of dynamic allocation) – opt */
    /* 서버의 인증서를 받는다. */
    server_cert = SSL_get_peer_certificate(ssl);
    CHK_NULL(server_cert);
    printf("Server certificate:\n");

    /* 인증서의 이름을 출력한다. */
    str = X509_NAME_oneline(X509_get_subject_name(server_cert), 0, 0);
    CHK_NULL(str);
    printf("t subject: %s\n", str);
    OPENSSL_free(str);

    /* 인증서의 issuer를 출력한다. */
    str = X509_NAME_oneline(X509_get_issuer_name(server_cert), 0, 0);
    CHK_NULL(str);
    printf("t issuer: %s\n", str);
    OPENSSL_free(str);

    /* We could do all sorts of certificate verification stuff here before deallocating the certificate */
    X509_free(server_cert);

	
 
		while(1) {
		memset(msg, 0x00, sizeof(msg));	
		memset(msg, 0x00, sizeof(recvmsg));
		printf("input message: ");
		scanf(" %s", msg);	
		SSL_write(ssl,msg, sizeof(msg));
		SSL_read(ssl, recvmsg, sizeof(msg));
		printf("recv message: %s\n", recvmsg);

	}
}	



