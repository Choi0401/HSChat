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
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <functional>
#include <netinet/in.h>
#include <sys/ipc.h>

#define BUFSIZE 6000
#define MAXLISTEN 1000
#define MAX_FD_SOCKET 512
#define MAX(a, b) a > b ? a : b

using namespace std;

int cnt_fd_socket;
void ShowCerts(SSL *ssl);

class Server
{

public:
	int serv_sock;

	~Server() { close(serv_sock); }
	int option, retcheck; // initialization
	char buf[BUFSIZE];
	const char *port = "7777";
	struct sockaddr_in serv_addr;
	socklen_t addr_len;
	fd_set reads, cpy_reads;
	int fd_socket[MAX_FD_SOCKET] = {0};
	int fd_max = {0};
};

class Client
{
public:
	int clnt_sock;
	char clnt_ip[32];
	char clnt_name[64];
};

int main()
{
	Server server;
	char sendBuf[BUFSIZE];
	struct timeval timeout;
	struct addrinfo ai, *ai_ret;
	sockaddr_in clnt_addr;
	vector<Client> c(MAX_FD_SOCKET);

	int i, read_len, rc_gai, errno, ret_check, fd, acc_cnt_sock, flag;

	/* SSL Context 및 관련 구조체를 선언한다. */
	SSL_CTX *ctx;
	SSL *ssl;
	X509 *client_cert;
	char *str;
	char buf[4096] = {
		0,
	};
	const SSL_METHOD *method;

	/* SSL 관련 초기화 작업을 수행한다. */
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	// method = TLSv1_1_server_method();
	// method = SSLv23_server_method();		// 서버 메소드.

	method = SSLv23_server_method(); // 서버 메소드.
	ctx = SSL_CTX_new(method);		 // 지정된 초기 값을 이용하여 SSL Context를 생성한다.

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
	// SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	// socket, bind, listen
	memset(server.buf, 0, sizeof(server.buf));

	for (i = 0; i < MAX_FD_SOCKET; i++)
	{
		c[i].clnt_sock = -1;
		memset(&c[i].clnt_ip, 0x00, sizeof(c[i].clnt_ip));
		memset(&c[i].clnt_name, 0x00, sizeof(c[i].clnt_name));
	}

	memset(&ai, 0, sizeof(ai));
	ai.ai_family = AF_INET;
	ai.ai_socktype = SOCK_STREAM;
	ai.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;

	if ((rc_gai = getaddrinfo(NULL, server.port, &ai, &ai_ret)) != 0)
	{
		printf("Fail: getaddrinfo(): %s", gai_strerror(rc_gai));
		exit(EXIT_FAILURE);
	}

	if ((server.serv_sock = socket(ai_ret->ai_family, ai_ret->ai_socktype, ai_ret->ai_protocol)) == -1)
	{
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	server.option = 1;
	server.retcheck = setsockopt(server.serv_sock, SOL_SOCKET, SO_REUSEADDR, &server.option, sizeof(server.option));
	if (server.retcheck == -1)
	{
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}

	if ((fcntl(server.serv_sock, F_SETFL, fcntl(server.serv_sock, F_GETFL) | O_NONBLOCK)) == -1)
	{
		perror("fcntl error");
		exit(EXIT_FAILURE);
	}

	if ((bind(server.serv_sock, ai_ret->ai_addr, ai_ret->ai_addrlen)) == -1)
	{
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if ((listen(server.serv_sock, MAXLISTEN)) == -1)
	{
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	// add socket
	if (cnt_fd_socket < MAX_FD_SOCKET)
	{
		c[cnt_fd_socket].clnt_sock = server.serv_sock;
		++cnt_fd_socket;
	}

	else
	{
		perror("add socket error");
		exit(EXIT_FAILURE);
	}

	i = 0;
	ret_check = 0;
	fd = 0;
	acc_cnt_sock = 0;

	while (1)
	{
		/* select함수의 경우 리턴하면서 fd_set타입을 변경시키기 때문에 매번 호출할 때마다 재작성하는 것을 막기 위해
	   복사본을 만들어 select에 넘길 때 복사본을 사용 */

		int i, fd_max;
		FD_ZERO(&server.reads); //파일 기술자 초기화
		for (i = 0, fd_max = -1; i < cnt_fd_socket; i++)
		{
			server.fd_max = MAX(fd_max, c[i].clnt_sock);
			FD_SET(c[i].clnt_sock, &server.reads);
		}

		if ((ret_check = select(server.fd_max + 1, &server.reads, NULL, NULL, NULL)) == -1)
		{
			perror("select error");
			break;
		}

		if (FD_ISSET(server.serv_sock, &server.reads))
		{
			server.addr_len = sizeof(clnt_addr);
			c[cnt_fd_socket].clnt_sock = accept(server.serv_sock, (struct sockaddr *)&clnt_addr, &server.addr_len);
			if (fd == -1)
			{
				if (errno == EAGAIN || errno == EINTR)
				{
					printf("Connecting Wait...");
					sleep(3);
				}

				else
				{
					perror("accept error");
					break;
				}
			}

			// add socket
			if (cnt_fd_socket < MAX_FD_SOCKET)
			{
				/* TCP connection is ready. Do server side SSL. */
				if ((ssl = SSL_new(ctx)) == NULL) // 설정된 Context를 이용하여 SSL 세션의 초기화 작업을 수행한다.
				{
					perror("SSL_new() error!");
				}

				strcpy(c[cnt_fd_socket].clnt_ip, inet_ntoa(clnt_addr.sin_addr));
				printf("accept: add socket(%d) (%s)\n", c[cnt_fd_socket].clnt_sock, c[cnt_fd_socket].clnt_ip);

				SSL_set_fd(ssl, c[cnt_fd_socket].clnt_sock);
				SSL_CTX_set_verify(SSL_get_SSL_CTX(ssl), SSL_VERIFY_PEER, NULL);

				if (SSL_accept(ssl) == -1) // SSL 세션을 통해 클라이언트의 접속을 대기한다.
				{
					perror("SSL_accept");
				}
				else
				{
					printf("SSL connection using %s \n", SSL_get_cipher(ssl));
					// ShowCerts(ssl);
				}

				/* 클라이언트의 인증서를 받음 – opt */
				client_cert = SSL_get_peer_certificate(ssl);
				if (client_cert != NULL)
				{
					printf("Client certificate:n");
				}

				if ((fcntl(c[cnt_fd_socket].clnt_sock, F_SETFL, fcntl(c[cnt_fd_socket].clnt_sock, F_GETFL) | O_NONBLOCK)) == -1)
				{
					perror("fcntl error");
					break;
				}

				++cnt_fd_socket;
			}

			else
			{
				perror("add socket error");
				exit(EXIT_FAILURE);
			}

			continue;
		}

		for (i = 1; i < cnt_fd_socket; i++)
		{
			if (FD_ISSET(c[i].clnt_sock, &server.reads))
			{
				read_len = SSL_read(ssl, server.buf, sizeof(server.buf));
				if (read_len == -1)
				{
					if (errno == EAGAIN || errno == EINTR)
					{ // 읽을 내용이 없거나 시그널이 발생하여 인터럽트 된 경우
						continue;
					}

					else
					{
						printf("Fail: recv (%d)\n", errno);
						exit(EXIT_FAILURE);
					}
				}

				else if (read_len == 0)
				{
					printf("client(%d): Session closed\n", c[i].clnt_sock);

					FD_CLR(c[i].clnt_sock, &server.reads);
					close(c[i].clnt_sock);
					SSL_free(ssl);
					SSL_CTX_free(ctx);
				}

				else
				{
					printf("recv(client(%d), size(%d), message(%s))\n", c[i].clnt_sock, read_len, server.buf);

					int write_len = SSL_write(ssl, server.buf, read_len);
					printf("Send Message %d bytes\n", write_len);
				}
			}
		}
	}
	server.~Server();
	return 0;
}

void ShowCerts(SSL *ssl)
{
	X509 *cert;
	char *line;

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
