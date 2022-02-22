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
#include <iostream>
#include "json/json.h"
#include <string>
#include <libpq-fe.h>

using namespace std;

#define	MAXLISTEN		  32	/* maximum connection queue length	*/
#define	BUFSIZE		4096
#define MAXCLI      100
#define MAXROOM		512

typedef struct data
{
	int size;
	string msg;
}Data;

extern int	errno;

int		createsocket(const char *portnum, int qlen);

class Server 
{

public:
    int serv_sock;

	~Server() { close(serv_sock); }
    int option, retcheck; // initialization
    char buf[BUFSIZE]; 
	const char* port = "8282";
    struct sockaddr_in serv_addr;
    socklen_t addr_len;	
};

class Client
{
public:
	int clnt_sock;
	int current_room_num;
	string clnt_ip;
	string clnt_name;
	string id;
	string nickname;
	string birth;

};

class CRoom
{
public:	
	int m_roomnum;
	int m_usernum;
	int m_maxnum;
	int m_masternum;
	string m_name;		
	string m_type;
	string m_master;	
	vector <Client> m_client;
};

//DML//
string DML_Insert(string args, ...);
string DML_Delete(int args, ...);
string DML_Select(int args, ...);
string DML_Update(int args, ...);



/*------------------------------------------------------------------------
 * main - Concurrent TCP server for ssl_read service
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
	const char	*portnum = "8282";	/* Standard server port number	*/
	struct sockaddr_in clnt_addr;	/* the from address of a client	*/
	int	serv_sock,r;			/* master server socket		*/
	fd_set	fd_reads;			/* read file descriptor set	*/
	fd_set	fd_cpy_reads;			/* active file descriptor set	*/
	unsigned int	addr_len;		/* from-address length		*/
	int	fd, nfds,i;

	vector<Client> c(MAXCLI);
/*-----------------------------------------------------------------------------*/
	/*------JSON Variable------*/
	Json::Value recvroot, sendroot;
	Json::StyledWriter writer;
	Json::Reader reader;
	int ret_HeadWrite;
	int ret_BodyWrite;
/*-----------------------------------------------------------------------------*/
	/*------DB Variable------ */
	PGconn *pCon = NULL;
	string DML;
	PGresult* res; 
	int nFields;
	string msg;
/*-----------------------------------------------------------------------------*/	

	// 메시지의 바디 크기를 구하기 위한 구조체
	Data data;
	
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
		c[i].current_room_num = 0;		// 대기실
		memset(&c[i].clnt_ip, 0x00, sizeof(c[i].clnt_ip));
		memset(&c[i].clnt_name, 0x00, sizeof(c[i].clnt_name));	
	}
	
    // tcp connection
    serv_sock = createsocket(portnum, MAXLISTEN);
    
	//max value of file descriptor 
	nfds = getdtablesize();

	FD_ZERO(&fd_cpy_reads);
	FD_SET(serv_sock, &fd_cpy_reads);


	pCon = PQsetdbLogin("192.168.1.125","8282", NULL, NULL, "postgres", "postgres", NULL);
	PQsetClientEncoding(pCon, "EUCKR");

	if(PQstatus(pCon) == CONNECTION_BAD)
	{
	cout << "Connection Fail"<<endl;
	return -1;	
	}	
	else
		cout << "Connection Succeed" << endl;



	
	int cntroom = 0;				// 채팅방 개수
	vector <CRoom> room(MAXROOM);
	for(int i = 0; i < MAXROOM; i++) {
		room[i].m_master = "";
		room[i].m_name = "";
		room[i].m_type = "";
		room[i].m_maxnum = 0;
		room[i].m_roomnum = 0;
		room[i].m_usernum = 0;
		room[i].m_masternum = 0;		
	}


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
			//strcpy(c[num_clients].clnt_ip, inet_ntoa(clnt_addr.sin_addr));
			c[num_clients].clnt_ip = inet_ntoa(clnt_addr.sin_addr);
			cout <<"client connected " << c[num_clients].clnt_sock <<" " << c[num_clients].clnt_ip << endl; 
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
				int clnt_sock;
	            // BIO struct
	            BIO *sbio = NULL;
	            for(i=0;i<num_clients;i++) {
	                if (fd_arr[i] == fd) {
	                    ssl = ssl_arr[i];
						clnt_sock = c[i].clnt_sock;
	                    index = i;
	                    break;
	                }
	            }
			    sbio = BIO_new_socket(fd, BIO_NOCLOSE);
                SSL_set_bio(ssl, sbio, sbio);
				int read_len = SSL_read(ssl, &data.size, sizeof(int)); 
				if (read_len == 0) {
				    fd_arr[index] = -1;
				    //SSL_shutdown(ssl);
				    //SSL_free(ssl);
				    (void) close(fd);
					printf("client closed(%d)\n", fd);
					FD_CLR(fd, &fd_cpy_reads);
				}

				else if (read_len > 0)
				{
					printf("Receive Body Size: %d\n", data.size);
					data.msg.resize(data.size);
					int ret_body_size = SSL_read(ssl, &data.msg[0], data.size);
					if (ret_body_size > 0)
					{
						cout << "Receive Message: "<< data.msg <<endl;
						bool parseSuccessful = reader.parse(data.msg, recvroot);
						if (parseSuccessful == false)
						{
							//std::cout << "Failed to parse configuration\n" << reader.getFormatedErrorMessages();
							return -1;						
						}

						else {
							string action = recvroot["action"].asString();
							if (action == "signup")
							{
								string id = recvroot["id"].asString();
								string pw = recvroot["pw"].asString();
								string nickname = recvroot["nickname"].asString();
								string name = recvroot["name"].asString();
								string birth = recvroot["birth"].asString();
								string phone = recvroot["phone"].asString();

								// 1. 아이디 중복 검사
								DML = DML_Select(4,"*","user_info","user_id",id.c_str());
								//cout << DML << endl;
								PGresult* resID = PQexec(pCon, DML.c_str()); //DML SEND;
								// 2. 닉네임 중복 검사
								DML = DML_Select(4,"*","user_info","user_nickname", nickname.c_str());
								PGresult* resNickname = PQexec(pCon, DML.c_str()); //DML SEND;

								//계정이 존재하지 않는 경우
								if (PQntuples(resID) == 0 && PQntuples(resNickname) == 0)  
								{
									DML = DML_Insert("user_info", name.c_str(), birth.c_str(), phone.c_str(), id.c_str(), nickname.c_str(), pw.c_str());
									PGresult* res = PQexec(pCon, DML.c_str()); //DML SEND

									sendroot["action"] = "signup";
									sendroot["result"] = "true";
									sendroot["msg"] = nickname + "님 환영합니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
														
								}	
								//이미 계정이 존재하는 경우
								else if(PQntuples(resID) > 0 || PQntuples(resNickname) > 0)
								{								
									cout << PQntuples(res) << endl;
									sendroot["action"] = "signup";
									sendroot["result"] = "false";
									if (PQntuples(resID) > 0)
										sendroot["msg"] = "이미 가입되어 있는 회원입니다";
									else if (PQntuples(resNickname) > 0)
										sendroot["msg"] = "이미 가입되어 있는 닉네임입니다";
																			
									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
						
								}	

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
										cout << "ret_HeadWrite_signup_error\n" <<endl;	

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_signup_error\n" << endl;																										  else 
									cout << "Send Success: " <<"("<< clnt_sock <<")"  << endl;			
								}								
							}	

							else if (action == "signin") 
							{
								string id = recvroot["id"].asString();
								string pw = recvroot["pw"].asString();
																
								// 아이디, 비밀번호 체크
								DML = DML_Select(6,"user_nickname","user_info","user_id",id.c_str(), "user_pw", pw.c_str());
								PGresult* rescheck = PQexec(pCon, DML.c_str()); //DML SEND;
																
								if (PQntuples(rescheck) == 0)  
								{
									sendroot["action"] = "signin";
									sendroot["result"] = "false";
									sendroot["msg"] = "아이디와 비밀번호를 확인해주세요";

								}
								else if(PQntuples(rescheck) == 1)
								{
									string nickname;
									nickname = PQgetvalue(rescheck, 0, 0);

									sendroot["action"] = "signin";
									sendroot["result"] = "true";
									sendroot["nickname"] = nickname;
									sendroot["msg"] = nickname + "님 환영합니다";
							
								}

								data.msg.clear();
								data.msg = writer.write(sendroot);
								data.size = data.msg.size();

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
										cout << "ret_HeadWrite_signup_error\n" <<endl;	

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_signup_error\n" << endl;																										  else 
									cout << "Send Success: " <<"("<< clnt_sock <<")"  << endl;			
								}	

							}	

							else if (action == "createroom") 
							{
								// parse json
								string master = recvroot["master"].asString();
								string roomname = recvroot["roomname"].asString();
								int maxnum = recvroot["maxnum"].asInt();
								string roomtype = recvroot["roomtype"].asString();

								//room[cntroom].m_roomnum = cntroom;
							
								/*										room_info TABLE
								*   room_num | room_master_user_num | room_num_user | room_max_user | room_name | room_type
								*   ----------+----------------------+---------------+---------------+-----------+---------
								*/

								// user_num Select
								DML = DML_Select(4,"user_num","user_info","user_nickname", master.c_str());								
								PGresult* resSelect = PQexec(pCon, DML.c_str());
								sendroot["action"] = "createroom";
								if(PQntuples(resSelect) == 1){
									room[cntroom].m_masternum = atoi(PQgetvalue(resSelect, 0, 0));
									//cout << room[cntroom].m_masternum << endl;		
									// Chatroom Insert						
									DML = DML_Insert("room_info", to_string(room[cntroom].m_masternum).c_str(), to_string(1).c_str(), to_string(maxnum).c_str(), roomname.c_str(), roomtype.c_str());									
									//cout << DML << endl;
									PGresult* resInsert = PQexec(pCon, DML.c_str()); 
									
									if(strlen(PQoidStatus(resInsert)) == 0){
										room[cntroom].m_masternum = 0;
										cout << "Insert ERROR" << endl;
										sendroot["result"] = "false";
										sendroot["msg"] = "방 만들기에 실패하였습니다";
									}
									else {									
										// room_num Select
										//DML = DML_Select(4,"room_num","room_info","room_master_user_num", room[cntroom].m_masternum);								
										DML = "select room_num from room_info where room_master_user_num = " + to_string(room[cntroom].m_masternum) + ";";
										PGresult* resSelect = PQexec(pCon, DML.c_str());
										if(PQntuples(resSelect) == 1){
											room[cntroom].m_roomnum = atoi(PQgetvalue(resSelect, 0, 0));
											cout << "채팅방 번호 : " << room[cntroom].m_roomnum << endl;							
											room[cntroom].m_master = master;
											cout << "채팅방 방장 : " << room[cntroom].m_master << endl;
											room[cntroom].m_name = roomname;
											cout << "채팅방 이름 : " << room[cntroom].m_name << endl;
											room[cntroom].m_type = roomtype;
											cout << "채팅방 타입: " << room[cntroom].m_type << endl;
											room[cntroom].m_usernum = 1;
											cout << "채팅방 현재 인원 : " << room[cntroom].m_usernum << endl;
											room[cntroom].m_maxnum = maxnum;
											cout << "채팅방 총 인원 : " << room[cntroom].m_maxnum << endl;
											room[cntroom].m_client.push_back(c[index]);
											cout << "채팅방 방장 IP: " << room[cntroom].m_client[room[cntroom].m_usernum - 1].clnt_ip << endl;	

											sendroot["result"] = "true";
											sendroot["roomnum"] = room[cntroom].m_roomnum;
											cntroom++;							
											data.msg.clear();
											data.msg = writer.write(sendroot);
											data.size = data.msg.size();										

											if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
												cout << "ret_HeadWrite_searchpw_error\n" <<endl;

											else // HeadWrite Successful
											{
												if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
													cout << "ret_BodyWrite_searchpw_error\n" << endl;
												cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
											}
											//TODO : 채팅방 만든거 대기실 유저에게 뿌려줘야함
										}										
										else {
											room[cntroom].m_masternum = 0;
											cout << "Insert ERROR" << endl;
											sendroot["result"] = "false";
											sendroot["msg"] = "방 만들기에 실패하였습니다";
										}
									}
								}
								else if(PQntuples(resSelect) == 0) {
									cout << "Cannot find user_num" << endl;
									sendroot["result"] = "false";
									sendroot["msg"] = "방 만들기에 실패하였습니다";
								} 


							}

							else if(action == "searchid") 
							{
								string name = recvroot["name"].asString();
								string birth = recvroot["birth"].asString();
								string phone = recvroot["phone"].asString();

								DML = DML_Select(8,"user_id","user_info","user_name",name.c_str(), "user_birth",birth.c_str(), "user_phone",phone.c_str());
								PGresult* resSearchID = PQexec(pCon, DML.c_str()); //DML SEND;
								if(PQntuples(resSearchID) > 0) // id를 찾은 경우
								{
									string id = PQgetvalue(resSearchID, 0,0);
									sendroot["action"] = "searchid";
									sendroot["result"] = "true";
									sendroot["msg"] = "아이디는" + id +  " 입니다";
									
									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
														
								}

								else if(PQntuples(resSearchID) == 0)// id를 찾지 못한 경우
								{
									sendroot["action"] = "searchid";
									sendroot["result"] = "false";
									sendroot["msg"] = "입력하신 정보가 일치하지 않습니다";
			
									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}	

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_searchid_error\n" <<endl;	

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_searchid_error\n" << endl;															 
									cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;			
								}								
															
							}/* end searchid */	

							else if (action == "searchpw")
							{
								string name = recvroot["name"].asString();
								string birth = recvroot["birth"].asString();
								string phone = recvroot["phone"].asString();
								string id = recvroot["id"].asString();

								DML = DML_Select(10,"user_pw","user_info","user_name",name.c_str(),
										"user_birth",birth.c_str(), "user_phone",phone.c_str(), "user_id", id.c_str());
								PGresult* resSearchPW = PQexec(pCon, DML.c_str()); //DML SEND;

								if(PQntuples(resSearchPW) > 0) // pw를 찾은 경우
								{
									string pw = PQgetvalue(resSearchPW, 0, 0);
									sendroot["action"] = "searchpw";
									sendroot["result"] = "true";
									sendroot["msg"] = "비밀번호는" + pw +  " 입니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								else if (PQntuples(resSearchPW) == 0)// id를 찾지 못한 경우
								{
									sendroot["action"] = "searchpw";
									sendroot["result"] = "false";
									sendroot["msg"] = "입력하신 정보가 일치하지 않습니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_searchpw_error\n" <<endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_searchpw_error\n" << endl;
									cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
								}

							}/* end searchpw */

						/*	else if (action == "showmyinfo")
							{
								string nickname = recvroot["nickname"].asString();
								DML = "Select " + "user_name" + "," + "user_id" + "," + "user_birth" + " from "
									+ "user_info" + " where " + "user_nickname" + " = " + "'" + nickname.c_str() + "'" +";";
								//다중 Select문 수정 필요
								PGresult* resShowMyInfo = PQexec(pCon, DML.c_str()); //DML SEND;

								if(PQntuples(resSearchShowMyInfo) > 0) // 일치하는 닉네임을 찾은 경우(내정보 출력)
								{
									string pw = PQgetvalue(resSearchPW, 0, 0);
									sendroot["action"] = "showmyinfo";
									sendroot["result"] = "true";
									sendroot["name"] = name;
									sendroot["id"] = id;
									sendroot["birth"] = birth;

								
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_showmyinfo_error\n" <<endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_showmyinfo_error\n" << endl;
									cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
								} */

						//	} /* end showmyinfo */
							

							
							
						
						
						}	


					
					
					
					}	
				
				
				
				}	

		}
	}
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

		int option;
		option = 1;
		

    /* Map port number (char string) to port number (int) */
        if ((serv_addr.sin_port=htons((unsigned short)atoi(portnum))) == 0)
                printf("can't get \"%s\" port number\n", portnum);

    /* Allocate a socket */
        serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serv_sock < 0)
            printf("can't create socket: %s\n", strerror(errno));

		setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );

    /* Bind the socket */
        if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            fprintf(stderr, "can't bind to %s port: %s; Trying other port\n",
                portnum, strerror(errno));
            serv_addr.sin_port=htons(0); /* request a port number to be allocated
                                   by bind */
		}
            else {
                socklen_t socklen = sizeof(serv_addr);                
			}
        

        if (listen(serv_sock, qlen) < 0)
            printf("can't listen on %s port: %s\n", portnum, strerror(errno));
        return serv_sock;
}







////////////////////////////////////////////////////////////////////////
/////////////////////////////////DML////////////////////////////////////
////////////////////////////////////////////////////////////////////////

string DML_Insert(string table, ...){
    string DML = "insert into " + table + " values (";
    va_list args;
	va_start(args, table);
    char * gData;
    
    if(table == "user_info")
    {
        DML += "nextval('sq_user_num'),'";//user_num : sequence
        for(int i = 0; i < 6; i++)
        {
            gData = va_arg(args, char *);
            if(i != 5)
            {
                DML += gData;
                DML += "','";
            }
            else
                DML += gData;
        }
        DML += "',0,0);";
    }
    
    
    else if(table == "friends_info")
    {
        DML += "'";
        for(int i = 0; i < 2; i++)
        {
            gData = va_arg(args, char *);
            if(i != 1)
            {
                DML += gData;
                DML += "','";          
            }
            else
                DML += gData;
        }
        DML += "');";
    }
    
    
    else if(table == "room_info")
    {
        DML += "nextval('room_num'),";//room_num : sequence
        for(int i = 0; i < 5; i++)
        {
            gData = va_arg(args, char *);
            if(i < 3)
            {
                DML += gData;
                DML += ",";
            }
            else if(i == 3)
            {
                DML += "'";
                DML += gData;
                DML += "',";
            }
            else
            {
                DML += "'";
                DML += gData;
                DML += "'";
            }
        }
        DML += ");";
    }
    va_end(args);

	return DML;
}

string DML_Select(int args, ...)
{
    string DML = "select ";
    

    va_list ap; 
    va_start(ap, args);

	char *gData;

	for(int i = 0; i < args; i++)
	{
		gData = va_arg(ap, char *);
		if(i == 0)
			DML += gData;
		else if(i == 1){
			DML += " from ";
			DML += gData;
			DML += " where ";
		}
		else
		{
			if(i % 2 == 0)
			{
				DML += gData;
				DML += " = '";
			}
			else if(i == args-1)
			{
				DML += gData;
				DML += "';";
			}
			else
			{
				DML += gData;
				DML += "' and ";
			}
		}
	}

    
    
    va_end(ap);
    return DML;
}


string DML_Delete(int args, ...)
{
	string DML = "delete from ";

	va_list ap;
	va_start(ap, args);

	char *gData;

	for(int i = 0; i < args; i++)
	{
		gData = va_arg(ap, char *);
		if(i == 0)
		{
			DML += gData;
			DML += " where ";
		}
		else
		{
			if(i%2 != 0)
			{
				DML += gData;
				DML += " = '";
			}
			else if(i == args - 1)
			{
				DML += gData;
				DML += "';";
			}
			else
			{
				DML += gData;
				DML += "' and ";
			}
		}
	}
	va_end(ap);
	return DML;
}

string DML_Update(int args, ...)
{
	string DML = "update ";

	va_list ap;
	va_start(ap, args);

	char *gData;

	for(int i = 0; i < args; i++)
	{
		gData = va_arg(ap, char *);
		if(i == 0)
		{
			DML += gData;
			DML += " set ";
		}
		else if(i == 1)
		{
			DML += gData;
			DML += " = '";
		}
		else if(i == 2)
		{
			DML += gData;
			DML += "' where ";
		}
		else
		{
			if(i % 2 == 1)
			{
				DML += gData;
				DML += " = '";
			}
			else if(i == args-1)
			{
				DML += gData;
				DML += "';";
			}
		}
	}
	va_end(ap);
	return DML;
}
