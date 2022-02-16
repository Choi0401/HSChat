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

#define BUFSIZE 6000
#define MAXLISTEN 1000
#define MAX_FD_SOCKET 512
#define MAX(a,b) a > b ? a : b

using namespace std;

int cnt_fd_socket;
int add_socket(int fd);
int del_socket(int fd);
int max_fd(fd_set* fd, int* fd_socket);

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
	fd_set reads, cpy_reads;
	int fd_socket[MAX_FD_SOCKET]={0};
	int fd_max={0};	
		
};

class Client
{
public:
	int clnt_sock;
	char clnt_ip[32];
	char clnt_name[64];

};

int main() {
    Server server;
	char sendBuf[BUFSIZE];
    struct timeval timeout;
    struct addrinfo ai, *ai_ret;
	sockaddr_in clnt_addr;
	vector <Client> c(MAX_FD_SOCKET);
	
    int i, read_len, rc_gai, errno, ret_check, fd, acc_cnt_sock, flag;
  
    // socket, bind, listen
    
	memset(server.buf, 0, sizeof(server.buf));

	for (i=0; i<MAX_FD_SOCKET; i++) 
	{
		c[i].clnt_sock = -1;
		memset(&c[i].clnt_ip, 0x00, sizeof(c[i].clnt_ip));
		memset(&c[i].clnt_name, 0x00, sizeof(c[i].clnt_name));	
	}

	memset(&ai, 0, sizeof(ai));
	ai.ai_family = AF_INET;
	ai.ai_socktype = SOCK_STREAM;
	ai.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;

	if ((rc_gai = getaddrinfo(NULL, server.port, &ai, &ai_ret ))!= 0)
	{
		printf("Fail: getaddrinfo(): %s", gai_strerror(rc_gai));
		exit(EXIT_FAILURE);
	
	}

	if((server.serv_sock = socket(ai_ret->ai_family, ai_ret->ai_socktype, ai_ret->ai_protocol)) == -1)
  	{
            perror("socket error");
            exit(EXIT_FAILURE);
    }

    server.option = 1;
    server.retcheck = setsockopt(server.serv_sock, SOL_SOCKET, SO_REUSEADDR, &server.option, sizeof(server.option));
    if (server.retcheck == -1) {
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}
	
	if((fcntl(server.serv_sock, F_SETFL, fcntl(server.serv_sock, F_GETFL) | O_NONBLOCK)) == -1)
	{
		perror("fcntl error");
		exit(EXIT_FAILURE);	
	}	
  	               
                	
    if((bind(server.serv_sock, ai_ret->ai_addr, ai_ret->ai_addrlen)) == -1)
	{
         perror("bind error");
         exit(EXIT_FAILURE);
    }
        
    if((listen(server.serv_sock, MAXLISTEN)) == -1)
	{	
		perror("listen error");
		exit(EXIT_FAILURE);	
	}	
     
	//add socket
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
   
    while(1) 
    {
        /* select함수의 경우 리턴하면서 fd_set타입을 변경시키기 때문에 매번 호출할 때마다 재작성하는 것을 막기 위해
       복사본을 만들어 select에 넘길 때 복사본을 사용 */

		int i, fd_max;
		FD_ZERO(&server.reads); //파일 기술자 초기화
		for (i=0, fd_max = -1; i < cnt_fd_socket; i++)
		{
			server.fd_max = MAX(fd_max, c[i].clnt_sock);
			FD_SET(c[i].clnt_sock, &server.reads);
		}	
		
		if ((ret_check = select(server.fd_max+1, &server.reads, NULL, NULL, NULL)) == -1)
		{
			perror("select error");
			break;
		}

		if (FD_ISSET(server.serv_sock, &server.reads))
		{
			server.addr_len = sizeof(clnt_addr);
		    fd = accept(server.serv_sock, (struct sockaddr*)&clnt_addr, &server.addr_len);
			if (fd == -1)
			{
				if(errno == EAGAIN || errno == EINTR)
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

			//add socket
			if (cnt_fd_socket < MAX_FD_SOCKET) 
			{
				c[cnt_fd_socket].clnt_sock = fd;
				strcpy(c[cnt_fd_socket].clnt_ip, inet_ntoa(clnt_addr.sin_addr));
				printf("accept: add socket(%d) (%s)\n", c[cnt_fd_socket].clnt_sock, c[cnt_fd_socket].clnt_ip);
				++cnt_fd_socket;
			}	
	
			else 
			{
				perror("add socket error");
				exit(EXIT_FAILURE);
			}

			if ((fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)) == -1)
			{
				perror("fcntl error");
				break;
			}
			
			continue;
										
		}
		
		for (i=1; i<cnt_fd_socket;i++)
		{
			if(FD_ISSET(c[i].clnt_sock, &server.reads))
			{
				read_len = recv(c[i].clnt_sock, server.buf, BUFSIZE, 0);
				if (read_len == -1) 
				{
					if(errno == EAGAIN || errno == EINTR) { // 읽을 내용이 없거나 시그널이 발생하여 인터럽트 된 경우
						continue;
					}

					else 
					{
						perror("Fail: recv");
						break;
					}					
				}

				else if (read_len == 0)
				{
					printf("client(%d): Session closed\n", c[i].clnt_sock);
					if (i != (cnt_fd_socket-1))
						c[i].clnt_sock = c[i].clnt_sock;
					FD_CLR(c[i].clnt_sock, &server.reads);
					close(c[i].clnt_sock);
				}

				else {
					printf("recv(client(%d), size(%d), message(%s))\n",c[i].clnt_sock, read_len, server.buf);				
				}				
			}	
		
		}

	}
server.~Server();
return 0;

}






