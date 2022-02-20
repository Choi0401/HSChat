#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <vector>


#define	MAXLISTEN		  32	/* maximum connection queue length	*/
#define	BUFSIZE		4096
#define MAXCLI      100

using namespace std;

extern int	errno;

int		createsocket(const char *portnum, int qlen);
int		ssl_read(SSL *ssl);

class Server 
{

public:
    int serv_sock;

	~Server() { close(serv_sock); }
    int option, retcheck; // initialization
    char buf[BUFSIZE]; 
	const char* port = "7777";
    struct sockaddr_in serv_addr;
    socklen_t addr_len;	
};

class Client
{
public:
	int clnt_sock;
	char clnt_ip[32];
	char clnt_name[64];

};

/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ssl_read service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	const char	*portnum = "7777";	/* Standard server port number	*/
	struct sockaddr_in clnt_addr;	/* the from address of a client	*/
	int	serv_sock,r;			/* master server socket		*/
	fd_set	fd_reads;			/* read file descriptor set	*/
	fd_set	fd_cpy_reads;			/* active file descriptor set	*/
	unsigned int	addr_len;		/* from-address length		*/
	int	fd, nfds,i;

	vector<Client> c(MAXCLI);
	
	// array of ssl structures
	SSL *ssl_arr[MAXCLI];
	
	// array of file descriptors
	int fd_arr[MAXCLI];
	
    // current number of client
    int num_clients=0;

	// client initialization
	for (i=0; i<MAXCLI; i++) 
	{
		c[i].clnt_sock = -1;
		memset(&c[i].clnt_ip, 0x00, sizeof(c[i].clnt_ip));
		memset(&c[i].clnt_name, 0x00, sizeof(c[i].clnt_name));	
	}
	
    // tcp connection
    serv_sock = createsocket(portnum, MAXLISTEN);
    
	//max value of file descriptor 
	nfds = getdtablesize();

	FD_ZERO(&fd_cpy_reads);
	FD_SET(serv_sock, &fd_cpy_reads);

	while (1) {
		memcpy(&fd_reads, &fd_cpy_reads, sizeof(fd_reads));

		if (select(nfds, &fd_reads, (fd_set *)0, (fd_set *)0,
				(struct timeval *)0) < 0)
			printf("select: %s\n", strerror(errno));
		if (FD_ISSET(serv_sock, &fd_reads)) {
		    // ssl method to be used
	        const SSL_METHOD *meth;
	
		    // ssl context structure
	        SSL_CTX *ctx;
	
	        // ssl struct
	        SSL *ssl;
	
	        // BIO struct
	        BIO *sbio = NULL;
	
	        // load encryption & hash algorithms for SSL
	        SSL_library_init();
	
	        // load the error strings for good error reporting            
            SSL_load_error_strings();
            
            // create context
            meth = SSLv23_server_method();
            ctx=SSL_CTX_new(meth);
            
            // server certificate
            if( SSL_CTX_use_certificate_file(ctx,"server.pem",SSL_FILETYPE_PEM) <= 0) {
                printf("Unable to load Server certificate\n");
            }
            // server private key
            if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
                printf("Unable to load Server Private Key\n");
            }
            // ssl initialize
            ssl = SSL_new(ctx);
			int	ssock;
			addr_len = sizeof(clnt_addr);
			ssock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &addr_len);
			sbio = BIO_new_socket(ssock, BIO_NOCLOSE);
            SSL_set_bio(ssl, sbio, sbio);
			if((r=SSL_accept(ssl))<=0) {
                printf("SSL accept failed\n%d\n",r);
            }
			if (ssock < 0)
				printf("accept: %s\n",
					strerror(errno));
			FD_SET(ssock, &fd_cpy_reads);
			ssl_arr[num_clients]=ssl;
			fd_arr[num_clients]=ssock;
			c[num_clients].clnt_sock = fd_arr[num_clients];
			strcpy(c[num_clients].clnt_ip, inet_ntoa(clnt_addr.sin_addr));
			printf("client connected(%d) (%s)\n", c[num_clients].clnt_sock, c[num_clients].clnt_ip);
			num_clients++;
			if(num_clients == MAXCLI) {
			    printf("Cant handle more than 100 clients");
			}
		}
		for (fd=0; fd<nfds; ++fd)
			if (fd != serv_sock && FD_ISSET(fd, &fd_reads)) {
			    // ssl struct
	            SSL *ssl;
	            int index;
	            // BIO struct
	            BIO *sbio = NULL;
	            for(i=0;i<num_clients;i++) {
	                if (fd_arr[i] == fd) {
	                    ssl = ssl_arr[i];
	                    index = i;
	                    break;
	                }
	            }
			    sbio = BIO_new_socket(fd, BIO_NOCLOSE);
                SSL_set_bio(ssl, sbio, sbio);
				if (ssl_read(ssl) == 0) {
				    fd_arr[index] = -1;
				    //SSL_shutdown(ssl);
				    //SSL_free(ssl);
				    (void) close(fd);
					printf("client closed(%d)\n", fd);
					FD_CLR(fd, &fd_cpy_reads);
				}

		}
	}
}

/*------------------------------------------------------------------------
 * ssl_read - echo one buffer of data, returning byte count
 *------------------------------------------------------------------------
 */
int ssl_read(SSL *ssl)
{
	int	read_len;
	char buf[BUFSIZE];

	memset(buf, 0x00, sizeof(buf));
	read_len = SSL_read(ssl, buf, sizeof(buf));

	if (read_len > 0)
		printf("receive message: %s\n", buf);
		
/*	
	if (cc && SSL_write(ssl, buf, cc) < 0)
		printf("ssl_read write: %s\n", strerror(errno)); */
	return read_len;
}



/*------------------------------------------------------------------------
 * createsocket - allocate & bind a server socket userv_addrg TCP
 *------------------------------------------------------------------------
 */
int createsocket(const char *portnum, int qlen)
/*
 * Arguments:
 *      portnum   - port number of the server
 *      qlen      - maximum server request queue length
 */
{
        struct sockaddr_in serv_addr; /* an Internet endpoint address  */
        int     serv_sock;              /* socket descriptor             */

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;

    /* Map port number (char string) to port number (int) */
        if ((serv_addr.sin_port=htons((unsigned short)atoi(portnum))) == 0)
                printf("can't get \"%s\" port number\n", portnum);

    /* Allocate a socket */
        serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serv_sock < 0)
            printf("can't create socket: %s\n", strerror(errno));

    /* Bind the socket */
        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            fprintf(stderr, "can't bind to %s port: %s; Trying other port\n",
                portnum, strerror(errno));
            serv_addr.sin_port=htons(0); /* request a port number to be allocated
                                   by bind */
            if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                printf("can't bind: %s\n", strerror(errno));
            else {
                socklen_t socklen = sizeof(serv_addr);

                if (getsockname(serv_sock, (struct sockaddr *)&serv_addr, &socklen) < 0)
                        printf("getsockname: %s\n", strerror(errno));
                printf("New server port number is %d\n", ntohs(serv_addr.sin_port));
            }
        }

        if (listen(serv_sock, qlen) < 0)
            printf("can't listen on %s port: %s\n", portnum, strerror(errno));
        return serv_sock;
}
