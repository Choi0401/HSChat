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
#include <locale.h>
#include <time.h>
#include "TSQueue.cpp"

using namespace std;

#define MAXLISTEN 32 /* maximum connection queue length	*/
#define BUFSIZE 4096
#define MAXCLI 100
#define MAXROOM 512

typedef struct data
{
	int size;
	string msg;
} Data;

extern int errno;

int createsocket(const char *portnum, int qlen);
void SendData(SSL *ssl, Data data);

class Server
{

public:
	int serv_sock;

	~Server() { close(serv_sock); }
	int option, retcheck; // initialization
	char buf[BUFSIZE];
	const char *port = "8282";
	struct sockaddr_in serv_addr;
	socklen_t addr_len;
};

class Client
{
public:
	int clnt_sock;
	int current_room_num;
	int usernum;
	bool ismaster;
	string clnt_ip;
	string clnt_name;
	string id;
	string nickname;
	string birth;
	SSL *clientssl;
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
	vector<Client> m_client;

	int m_getindex();
};

// DML//
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
	setlocale(LC_ALL, "ko_KR.EUC-KR");
	const char *portnum = "8282"; /* Standard server port number	*/
	struct sockaddr_in clnt_addr; /* the from address of a client	*/
	int serv_sock, r;			  /* master server socket		*/
	fd_set fd_reads;			  /* read file descriptor set	*/
	fd_set fd_cpy_reads;		  /* active file descriptor set	*/
	unsigned int addr_len;		  /* from-address length		*/
	int fd, nfds, i;

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
	PGresult *res;
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
	int num_clients = 0;

	// client initialization
	for (i = 0; i < MAXCLI; i++)
	{
		c[i].clnt_sock = -1;
		c[i].current_room_num = 0; // 대기실
		c[i].ismaster = false;
		c[i].clientssl = NULL;
		c[i].usernum = 0;
		memset(&c[i].clnt_ip, 0x00, sizeof(c[i].clnt_ip));
		memset(&c[i].clnt_name, 0x00, sizeof(c[i].clnt_name));
	}

	// tcp connection
	serv_sock = createsocket(portnum, MAXLISTEN);

	// max value of file descriptor
	nfds = getdtablesize();

	FD_ZERO(&fd_cpy_reads);
	FD_SET(serv_sock, &fd_cpy_reads);

	pCon = PQsetdbLogin("192.168.1.125", "8282", NULL, NULL, "postgres", "postgres", NULL);
	PQsetClientEncoding(pCon, "UTF8");

	if (PQstatus(pCon) == CONNECTION_BAD)
	{
		cout << "Connection Fail" << endl;
		return -1;
	}
	else
		cout << "Connection Succeed" << endl;

	int cntroom = 0; // 채팅방 개수
	vector<CRoom> room(MAXROOM);
	for (int i = 0; i < MAXROOM; i++)
	{
		room[i].m_master = "";
		room[i].m_name = "";
		room[i].m_type = "";
		room[i].m_maxnum = 0;
		room[i].m_roomnum = 0;
		room[i].m_usernum = 0;
		room[i].m_masternum = 0;
	}

	TsQueue<string> tsqueue;
	while (1)
	{
		memcpy(&fd_reads, &fd_cpy_reads, sizeof(fd_reads));

		if (select(nfds, &fd_reads, (fd_set *)0, (fd_set *)0,
				   (struct timeval *)0) < 0)
			printf("select: %s\n", strerror(errno));
		if (FD_ISSET(serv_sock, &fd_reads))
		{
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
			ctx = SSL_CTX_new(meth);

			// server certificate
			if (SSL_CTX_use_certificate_file(ctx, "server.pem", SSL_FILETYPE_PEM) <= 0)
			{
				printf("Unable to load Server certificate\n");
			}
			// server private key
			if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
			{
				printf("Unable to load Server Private Key\n");
			}
			// ssl initialize
			ssl = SSL_new(ctx);
			int ssock;
			addr_len = sizeof(clnt_addr);
			ssock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &addr_len);
			sbio = BIO_new_socket(ssock, BIO_NOCLOSE);
			SSL_set_bio(ssl, sbio, sbio);
			if ((r = SSL_accept(ssl)) <= 0)
			{
				printf("SSL accept failed\n%d\n", r);
			}
			if (ssock < 0)
				printf("accept: %s\n",
					   strerror(errno));
			FD_SET(ssock, &fd_cpy_reads);
			ssl_arr[num_clients] = ssl;

			c[num_clients].clientssl = ssl;

			fd_arr[num_clients] = ssock;
			c[num_clients].clnt_sock = fd_arr[num_clients];
			// strcpy(c[num_clients].clnt_ip, inet_ntoa(clnt_addr.sin_addr));
			c[num_clients].clnt_ip = inet_ntoa(clnt_addr.sin_addr);
			cout << "client connected " << c[num_clients].clnt_sock << " " << c[num_clients].clnt_ip << endl;
			num_clients++;
			if (num_clients == MAXCLI)
			{
				printf("Cant handle more than 100 clients");
			}
		}
		for (fd = 0; fd < nfds; ++fd)
			if (fd != serv_sock && FD_ISSET(fd, &fd_reads))
			{
				// ssl struct
				SSL *ssl;
				int index;
				int clnt_sock;
				// BIO struct
				BIO *sbio = NULL;
				for (i = 0; i < num_clients; i++)
				{
					if (fd_arr[i] == fd)
					{
						ssl = ssl_arr[i];
						clnt_sock = c[i].clnt_sock;
						index = i;
						break;
					}
				}
				sbio = BIO_new_socket(fd, BIO_NOCLOSE);
				SSL_set_bio(ssl, sbio, sbio);
				int read_len = SSL_read(ssl, &data.size, sizeof(int));
				if (read_len == 0)
				{
					fd_arr[index] = -1;
					// SSL_shutdown(ssl);
					// SSL_free(ssl);
					(void)close(fd);
					printf("client closed(%d)\n", fd);
					FD_CLR(fd, &fd_cpy_reads);
				}

				else if (read_len > 0)
				{
					printf("Receive Body Size: %d\n", data.size);
					data.msg.resize(data.size);
					int ret_body_size = SSL_read(ssl, &data.msg[0], data.size);
					// tsqueue.push(data.msg);

					if (ret_body_size > 0)
					{
						cout << "Receive Message: " << data.msg << endl;
						bool parseSuccessful = reader.parse(data.msg, recvroot);
						if (parseSuccessful == false)
						{
							// std::cout << "Failed to parse configuration\n" << reader.getFormatedErrorMessages();
							return -1;
						}

						else
						{
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
								DML = DML_Select(4, "*", "user_info", "user_id", id.c_str());
								// cout << DML << endl;
								PGresult *resID = PQexec(pCon, DML.c_str()); // DML SEND;

								if (PQresultStatus(resID) == PGRES_TUPLES_OK)
								{
									// 2. 닉네임 중복 검사
									DML = DML_Select(4, "*", "user_info", "user_nickname", nickname.c_str());
									PGresult *resNickname = PQexec(pCon, DML.c_str()); // DML SEND;
									if (PQresultStatus(resNickname) == PGRES_TUPLES_OK)
									{
										cout << PQntuples(resID) << " " << PQntuples(resNickname) << endl;
										//계정이 존재하지 않는 경우
										if (PQntuples(resID) == 0 && PQntuples(resNickname) == 0)
										{
											DML = "insert into user_info values(nextval('sq_user_num'),'" + name + "','" + birth + "','" + phone + "','" + id + "','" + nickname + "','" + pw + "',0,0);";
											PGresult *resInsert = PQexec(pCon, DML.c_str()); // DML SEND

											if (PQresultStatus(resInsert) == PGRES_COMMAND_OK)
											{
												sendroot["action"] = "signup";
												sendroot["result"] = "true";
												sendroot["msg"] = nickname + "님 환영합니다";

												DML = DML_Select(4, "user_num", "user_info", "user_nickname", nickname.c_str());
												PGresult *resSelect = PQexec(pCon, DML.c_str()); // DML SEND
												if (PQntuples(resSelect) == 1)
												{
													c[index].usernum = atoi(PQgetvalue(resSelect, 0, 0));
													cout << "usernum = " << c[index].usernum << endl;
												}
											}
											else
											{
												cout << "Insert ERROR" << endl;
												sendroot["result"] = "false";
												sendroot["msg"] = "회원가입에 실패하였습니다";
												cout << PQerrorMessage(pCon) << endl;
											}
										}
										//이미 계정이 존재하는 경우
										else if (PQntuples(resID) > 0 || PQntuples(resNickname) > 0)
										{
											sendroot["action"] = "signup";
											sendroot["result"] = "false";
											if (PQntuples(resID) > 0)
												sendroot["msg"] = "이미 가입되어 있는 회원입니다";
											else if (PQntuples(resNickname) > 0)
												sendroot["msg"] = "이미 가입되어 있는 닉네임입니다";
										}

										/* Json Data Send */
										data.msg.clear();
										data.msg = writer.write(sendroot);
										data.size = data.msg.size();
									}
									else
									{
										cout << PQerrorMessage(pCon) << endl;
									}
									PQclear(resNickname);
								}
								else
								{
									cout << PQerrorMessage(pCon) << endl;
								}
								PQclear(resID);
							}

							else if (action == "signin")
							{
								string id = recvroot["id"].asString();
								string pw = recvroot["pw"].asString();

								// 아이디, 비밀번호 체크
								DML = DML_Select(6, "user_nickname", "user_info", "user_id", id.c_str(), "user_pw", pw.c_str());
								PGresult *rescheck = PQexec(pCon, DML.c_str()); // DML SEND;

								if (PQresultStatus(rescheck) == PGRES_TUPLES_OK)
								{

									if (PQntuples(rescheck) == 0)
									{
										sendroot["action"] = "signin";
										sendroot["result"] = "false";
										sendroot["msg"] = "아이디와 비밀번호를 확인해주세요";
									}
									else if (PQntuples(rescheck) == 1)
									{
										string nickname;
										nickname = PQgetvalue(rescheck, 0, 0);
										c[index].nickname = nickname;

										cout << nickname << endl;

										sendroot["action"] = "signin";
										sendroot["result"] = "true";
										sendroot["nickname"] = nickname;
										sendroot["msg"] = nickname + "님 환영합니다";
									}
								}
								else
								{
									cout << PQerrorMessage(pCon) << endl;
								}
								PQclear(rescheck);

								data.msg.clear();
								data.msg = writer.write(sendroot);
								data.size = data.msg.size();
							}

							else if (action == "createroom")
							{
								// parse json
								string master = recvroot["master"].asString();
								string roomname = recvroot["roomname"].asString();
								int maxnum = recvroot["maxnum"].asInt();
								string roomtype = recvroot["roomtype"].asString();

								// room[cntroom].m_roomnum = cntroom;

								/*										room_info TABLE
								 *   room_num | room_master_user_num | room_num_user | room_max_user | room_name | room_type
								 *   ----------+----------------------+---------------+---------------+-----------+---------
								 */

								// get user_num
								DML = DML_Select(4, "user_num", "user_info", "user_nickname", master.c_str());
								PGresult *resSelect = PQexec(pCon, DML.c_str());

								if (PQresultStatus(resSelect) == PGRES_TUPLES_OK)
								{
									sendroot["action"] = "createroom";
									if (PQntuples(resSelect) == 1)
									{
										room[cntroom].m_masternum = atoi(PQgetvalue(resSelect, 0, 0));
										// cout << room[cntroom].m_masternum << endl;
										//  Chatroom Insert
										DML = DML_Insert("room_info", to_string(room[cntroom].m_masternum).c_str(), to_string(1).c_str(), to_string(maxnum).c_str(), roomname.c_str(), roomtype.c_str());
										// cout << DML << endl;
										PGresult *resInsert = PQexec(pCon, DML.c_str());

										if (PQresultStatus(resInsert) == PGRES_COMMAND_OK)
										{
											// get room_num
											DML = "select room_num from room_info where room_master_user_num = " + to_string(room[cntroom].m_masternum) + ";";
											PGresult *resSelect = PQexec(pCon, DML.c_str());

											if (PQresultStatus(resSelect) == PGRES_TUPLES_OK)
											{
												if (PQntuples(resSelect) == 1)
												{
													room[cntroom].m_roomnum = atoi(PQgetvalue(resSelect, 0, 0));
													c[index].current_room_num = room[cntroom].m_roomnum;
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
													room[cntroom].m_client[room[cntroom].m_usernum - 1].ismaster = true;

													// cout << room[cntroom].m_client[room[cntroom].m_usernum - 1].nickname << endl;

													// update current_room_num
													DML = "update user_info set current_room_num = " + to_string(room[cntroom].m_roomnum) + " where user_num = " + to_string(room[cntroom].m_masternum) + ";";
													PGresult *resUpdate = PQexec(pCon, DML.c_str());

													if (PQresultStatus(resUpdate) == PGRES_COMMAND_OK)
													{

														sendroot["result"] = "true";
														sendroot["roomnum"] = room[cntroom].m_roomnum;
														cntroom++;

														// TODO : 채팅방 만든거 대기실 유저에게 뿌려줘야함
													}
													else
													{
														cout << PQerrorMessage(pCon) << endl;
													}
												}
											}
											else
											{
												cout << PQerrorMessage(pCon) << endl;
											}
										}

										else
										{
											cout << PQerrorMessage(pCon) << endl;
											room[cntroom].m_masternum = 0;
											cout << "Insert ERROR" << endl;
											sendroot["result"] = "false";
											sendroot["msg"] = "방 만들기에 실패하였습니다";
										}
									}
									else if (PQntuples(resSelect) == 0)
									{
										cout << "Cannot find user_num" << endl;
										sendroot["result"] = "false";
										sendroot["msg"] = "방 만들기에 실패하였습니다";
									}
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}
							}

							else if (action == "alllist")
							{
								/*										room_info TABLE
								 *   room_num | room_master_user_num | room_num_user | room_max_user | room_name | room_type
								 *   ----------+----------------------+---------------+---------------+-----------+---------
								 */
								string nickname = recvroot["nickname"].asString();
								DML = "select * from room_info;";
								PGresult *resSelect = PQexec(pCon, DML.c_str());

								if (PQresultStatus(resSelect) == PGRES_TUPLES_OK)
								{

									int cnttuples = PQntuples(resSelect);
									if (cnttuples >= 0)
									{
										cout << "asdfdsa" << endl;
										Json::Value roomlist;
										Json::Value roomarr[cnttuples];

										for (int i = 0; i < cnttuples; i++) // Get tables data
										{
											roomarr[i]["roomnum"] = stoi(PQgetvalue(resSelect, i, 0));
											roomarr[i]["usernum"] = stoi(PQgetvalue(resSelect, i, 2));
											roomarr[i]["maxusernum"] = stoi(PQgetvalue(resSelect, i, 3));
											roomarr[i]["roomname"] = PQgetvalue(resSelect, i, 4);
											roomarr[i]["roomtype"] = PQgetvalue(resSelect, i, 5);
											roomlist.append(roomarr[i]);
										}
										sendroot["action"] = "alllist";
										sendroot["result"] = "true";
										sendroot["roomlist"] = roomlist;

										data.msg.clear();
										data.msg = writer.write(sendroot);
										data.size = data.msg.size();
									}
								}
								else
								{
									cout << PQerrorMessage(pCon) << endl;
								}
							}
							else if (action == "quitroom")
							{
								string nickname = recvroot["nickname"].asString();
								// Get user_num
								DML = DML_Select(4, "user_num", "user_info", "user_nickname", nickname.c_str());
								PGresult *resSelect = PQexec(pCon, DML.c_str());

								if (PQresultStatus(resSelect) == PGRES_TUPLES_OK)
								{
									if (PQntuples(resSelect) == 1)
									{
										int user_num = atoi(PQgetvalue(resSelect, 0, 0));
										// Get room info
										DML = "select * from room_info;";
										PGresult *resSelect = PQexec(pCon, DML.c_str());

										if (PQresultStatus(resSelect) == PGRES_TUPLES_OK)
										{

											int cnttuples = PQntuples(resSelect);

											if (cnttuples > 0)
											{
												int roomnum, currentusernum, masterusernum;
												for (int i = 0; i < cnttuples; i++) // Get tables data
												{
													roomnum = stoi(PQgetvalue(resSelect, i, 0));
													masterusernum = stoi(PQgetvalue(resSelect, i, 1));
													currentusernum = stoi(PQgetvalue(resSelect, i, 2));
												}

												// 방에 혼자 남은 경우
												if (currentusernum == 1)
												{
													for (int i = 0; i < room.size(); i++)
													{
														if (room[i].m_roomnum == roomnum)
														{
															room.erase(room.begin() + i);
															break;
														}
													}
													// 사용자 채팅방 번호 업데이트
													DML = "update user_info set current_room_num = " + to_string(0) + " where user_num = " + to_string(user_num) + ";";
													PGresult *resUpdate = PQexec(pCon, DML.c_str());

													if (PQresultStatus(resUpdate) == PGRES_COMMAND_OK)
													{
														// 채팅방 DB에서 삭제
														DML = "delete from room_info where room_num = " + to_string(roomnum) + ";";
														PGresult *resDelete = PQexec(pCon, DML.c_str());
														cout << PQresultStatus(resDelete) << endl;
													}
													else
													{
														cout << PQerrorMessage(pCon) << endl;
													}
													c[index].current_room_num = 0;
													continue;
												}
												else if (currentusernum > 1)
												{
													int roomindex = 0, clientindex = 0;
													// 방 인덱스
													for (int i = 0; i < room.size(); i++)
													{
														if (room[i].m_roomnum == roomnum)
														{
															roomindex = i;
															room[i].m_usernum--;
															break;
														}
													}
													// 클라이언트 인덱스
													for (int j = 0; j < room[roomindex].m_client.size(); j++)
													{
														if (room[roomindex].m_client[j].nickname == nickname)
															clientindex = j;
													}
													// 나간사람이 방장인 경우 방장 위임
													if (masterusernum == user_num)
													{
														DML = "select user_num from user_info inner join room_info on user_info.current_room_num = room_info.room_num where user_info.user_num != room_info.room_master_user_num and user_info.current_room_num = " + to_string(room[roomindex].m_roomnum) + ";";
														// cout << DML << endl;
														PGresult *resselectuser = PQexec(pCon, DML.c_str());

														if (PQresultStatus(resselectuser) == PGRES_TUPLES_OK)
														{
															int nextusernum = atoi(PQgetvalue(resselectuser, 0, 0));
															DML = "update room_info set room_master_user_num = " + to_string(nextusernum) + " where room_num = " + to_string(room[roomindex].m_roomnum) + ";";
															PGresult *resUpdateroom = PQexec(pCon, DML.c_str());

															for (int i = 0; i < room[roomindex].m_client.size(); i++)
															{
																if (room[roomindex].m_client[i].usernum == nextusernum)
																{
																	room[roomindex].m_client[i].ismaster = true;
																}
															}
															room[roomindex].m_client[clientindex].ismaster = false;
															// 사용자 채팅방 번호 업데이트
															DML = "update user_info set current_room_num = " + to_string(0) + " where user_num = " + to_string(user_num) + ";";
															PGresult *resUpdateuser = PQexec(pCon, DML.c_str());
															// 채팅방 현재 인원수 업데이트
															DML = "update room_info set room_num_user = " + to_string(room[roomindex].m_usernum) + " where room_num = " + to_string(roomnum) + ";";
															resUpdateroom = PQexec(pCon, DML.c_str());
															nickname += "(방장)";

															currentusernum--;
														}
														else
															cout << PQerrorMessage(pCon) << endl;
													}
													else
													{
														// 사용자 채팅방 번호 업데이트
														DML = "update user_info set current_room_num = " + to_string(0) + " where user_num = " + to_string(user_num) + ";";
														PGresult *resUpdateuser = PQexec(pCon, DML.c_str());
														// 채팅방 현재 인원수 업데이트
														DML = "update room_info set room_num_user = " + to_string(room[roomindex].m_usernum) + " where room_num = " + to_string(roomnum) + ";";
														PGresult *resUpdateroom = PQexec(pCon, DML.c_str());
														cout << "Client cnt = " << room[roomindex].m_client.size() << endl;
														room[roomindex].m_client.erase(room[roomindex].m_client.begin() + clientindex);
														// for (int j = 0; j < room[roomindex].m_client.size(); j++)
														//{
														// if (room[roomindex].m_client[j].nickname == nickname)
														// room[roomindex].m_client.erase(room[roomindex].m_client.begin() + j);
														//}
														cout << "Client cnt = " << room[roomindex].m_client.size() << endl;
														currentusernum--;
													}
													c[index].current_room_num = 0;
													/* 같은 채팅방 유저에게 알려줌 */
													sendroot.clear();
													sendroot["action"] = "updateuserlist";
													sendroot["inout"] = "out";
													sendroot["nickname"] = nickname;
													string sendmsg = "[공지]" + nickname + "님이 채팅방에서 퇴장하였습니다.\r\n";
													sendroot["msg"] = sendmsg;

													data.msg.clear();
													data.msg = writer.write(sendroot);
													data.size = data.msg.size();

													int cnt_client = room[roomindex].m_client.size();
													for (int i = 0; i < cnt_client; i++)
													{
														if (room[roomindex].m_client[i].nickname != nickname)
														{
															cout << "test" << endl;
															SendData(room[roomindex].m_client[i].clientssl, data);
														}
													}
													continue;
												}
											}
										}
										else
										{
											cout << PQerrorMessage(pCon) << endl;
										}
									}
								}
								else
								{
									cout << PQerrorMessage(pCon) << endl;
								}
							}

							else if (action == "enterroom")
							{
								string nickname = recvroot["nickname"].asString();
								int roomnum = recvroot["roomnum"].asInt();

								int roomindex = 0;
								for (int i = 0; i < room.size(); i++)
								{
									if (roomnum == room[i].m_roomnum)
									{
										roomindex = i;
										break;
									}
								}

								DML = "select room_num from room_info where room_num = " + to_string(roomnum) + ";";
								PGresult *resSelecteNum = PQexec(pCon, DML.c_str());

								if (PQntuples(resSelecteNum) == 0)
								{
									sendroot["action"] = "enterroom";
									sendroot["result"] = "false";
									sendroot["msg"] = "채팅방에 입장할 수 없습니다";

									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								DML = "select room_num_user, room_max_user from room_info where room_num = " + to_string(roomnum) + ";";
								resSelecteNum = PQexec(pCon, DML.c_str());
								string room_num_user = PQgetvalue(resSelecteNum, 0, 0);
								string room_max_user = PQgetvalue(resSelecteNum, 0, 1);

								if (room_num_user == room_max_user)
								{
									sendroot["action"] = "enterroom";
									sendroot["result"] = "false";
									sendroot["msg"] = "채팅방이 꽉찼습니다.";

									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}
								else
								{
									// 채팅방 현재 인원 수 업데이트
									DML = "update room_info set room_num_user = " + to_string(++room[roomindex].m_usernum) + " where room_num = " + to_string(roomnum) + ";";
									PGresult *resUpdateNum = PQexec(pCon, DML.c_str()); // DML SEND;
									room[roomindex].m_client.push_back(c[index]);

									// 클라이언트 현재 채팅방 번호 업데이트
									DML = "update user_info set current_room_num = " + to_string(roomnum) + " where user_nickname = " + "'" + nickname + "'" + ";";
									PGresult *resUpdateroomnum = PQexec(pCon, DML.c_str()); // DML SEND;

									// cout << "num = " << room[roomindex].m_usernum << endl;
									int cntuser = room[roomindex].m_client.size();
									// cout << "cntuser = " << cntuser << endl;
									// cout << "master = " << room[roomindex].m_master << endl;

									Json::Value userlist;
									Json::Value user[cntuser];
									sendroot["action"] = "enterroom";
									sendroot["result"] = "true";
									sendroot["roomnum"] = roomnum;
									sendroot["master"] = room[roomindex].m_master;
									sendroot["msg"] = "채팅방에 오신 것을 환영합니다.\r\n\r\n";
									for (int i = 0; i < cntuser; i++)
									{
										user[i]["nickname"] = room[roomindex].m_client[i].nickname;
										cout << "nickname = " << room[roomindex].m_client[i].nickname << endl;
										userlist.append(user[i]);
									}
									sendroot["userlist"] = userlist;

									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
									SendData(ssl, data);

									/* 같은 채팅방 유저에게 알려줌 */
									sendroot.clear();
									sendroot["action"] = "updateuserlist";
									sendroot["inout"] = "in";
									sendroot["nickname"] = nickname;
									string sendmsg = "[공지]" + nickname + "님이 채팅방에 입장하였습니다.\r\n";
									sendroot["msg"] = sendmsg;

									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();

									int cnt_client = room[roomindex].m_client.size();
									for (int i = 0; i < cnt_client; i++)
									{
										if (room[roomindex].m_client[i].nickname != nickname)
										{
											SendData(room[roomindex].m_client[i].clientssl, data);
										}
									}
									continue;
								}
							}

							else if (action == "sendmsg")
							{
								string nickname = recvroot["nickname"].asString();
								int roomnum = recvroot["roomnum"].asInt();
								string msg = recvroot["msg"].asString();
								string receiver = recvroot["receiver"].asString();

								int roomindex = 0;
								for (int i = 0; i < room.size(); i++)
								{
									if (roomnum == room[i].m_roomnum)
									{
										roomindex = i;
										break;
									}
								}

								time_t timer;
								struct tm *t;
								timer = time(NULL);	   // 1970년 1월 1일 0시 0분 0초부터 시작하여 현재까지의 초
								t = localtime(&timer); // 포맷팅을 위해 구조체에 넣기
								string time;
								time = to_string(t->tm_hour) + ":" + to_string(t->tm_min);

								sendroot["action"] = "recvmsg";
								sendroot["sender"] = nickname;
								sendroot["time"] = time;
								sendroot["msg"] = msg;

								if (receiver == "all")
								{
									sendroot["iswhisper"] = "false";
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();

									int cnt_client = room[roomindex].m_client.size();
									for (int i = 0; i < cnt_client; i++)
									{
										SendData(room[roomindex].m_client[i].clientssl, data);
										cout << "Send to " << room[roomindex].m_client[i].clnt_ip << endl;
									}
									continue;
								}
								else
								{
									sendroot["iswhisper"] = "true";
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();

									int cnt_client = room[roomindex].m_client.size();
									for (int i = 0; i < cnt_client; i++)
									{
										if (room[roomindex].m_client[i].nickname == receiver || room[roomindex].m_client[i].nickname == nickname)
										{
											SendData(room[roomindex].m_client[i].clientssl, data);
											cout << "Send to " << room[roomindex].m_client[i].nickname << endl;
										}
									}
									continue;
								}
							}

							else if (action == "searchid")
							{
								string name = recvroot["name"].asString();
								string birth = recvroot["birth"].asString();
								string phone = recvroot["phone"].asString();

								DML = DML_Select(8, "user_id", "user_info", "user_name", name.c_str(), "user_birth", birth.c_str(), "user_phone", phone.c_str());
								PGresult *resSearchID = PQexec(pCon, DML.c_str()); // DML SEND;
								if (PQntuples(resSearchID) > 0)					   // id를 찾은 경우
								{
									string id = PQgetvalue(resSearchID, 0, 0);
									sendroot["action"] = "searchid";
									sendroot["result"] = "true";
									sendroot["msg"] = "아이디는" + id + " 입니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								else if (PQntuples(resSearchID) == 0) // id를 찾지 못한 경우
								{
									sendroot["action"] = "searchid";
									sendroot["result"] = "false";
									sendroot["msg"] = "입력하신 정보가 일치하지 않습니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

							} /* end searchid */

							else if (action == "searchpw")
							{
								string name = recvroot["name"].asString();
								string birth = recvroot["birth"].asString();
								string phone = recvroot["phone"].asString();
								string id = recvroot["id"].asString();

								DML = DML_Select(10, "user_pw", "user_info", "user_name", name.c_str(),
												 "user_birth", birth.c_str(), "user_phone", phone.c_str(), "user_id", id.c_str());
								PGresult *resSearchPW = PQexec(pCon, DML.c_str()); // DML SEND;

								if (PQntuples(resSearchPW) > 0) // pw를 찾은 경우
								{
									string pw = PQgetvalue(resSearchPW, 0, 0);
									sendroot["action"] = "searchpw";
									sendroot["result"] = "true";
									sendroot["msg"] = "비밀번호는" + pw + " 입니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								else if (PQntuples(resSearchPW) == 0) // id를 찾지 못한 경우
								{
									sendroot["action"] = "searchpw";
									sendroot["result"] = "false";
									sendroot["msg"] = "입력하신 정보가 일치하지 않습니다";

									/* Json Data Send */
									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

							} /* end searchpw */

							else if (action == "showmyinfo")
							{
								const char *gdata;
								string nickname = recvroot["nickname"].asString();
								DML = "select user_name, user_id, user_birth from user_info where user_nickname = ";
								gdata = "'";
								DML += gdata;
								DML += nickname;
								DML += gdata;
								gdata = ";";
								DML += gdata;

								+"'" + nickname + "'" + ";";

								PGresult *resshowmyinfo = PQexec(pCon, DML.c_str()); // DML SEND;

								string name = PQgetvalue(resshowmyinfo, 0, 0);
								string id = PQgetvalue(resshowmyinfo, 0, 1);
								string birth = PQgetvalue(resshowmyinfo, 0, 2);

								if (PQntuples(resshowmyinfo) > 0) // 일치하는 닉네임을 찾은 경우(내정보 출력)
								{
									sendroot["action"] = "showmyinfo";
									sendroot["result"] = "true";
									sendroot["name"] = name;
									sendroot["id"] = id;
									sendroot["birth"] = birth;
									sendroot["nickname"] = nickname;

									data.msg.clear();
									data.msg = writer.write(sendroot);
									data.size = data.msg.size();
								}

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_showmyinfo_error\n"
										 << endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_showmyinfo_error\n"
											 << endl;
									cout << "Send Success: "
										 << "(" << c[index].clnt_sock << ")" << endl;
								}

							} /* end showmyinfo */

							else if (action == "addfriend")
							{
								int getfriendnum;
								int friendcnt = 0;
								string nickname = recvroot["nickname"].asString();
								string fnickname = recvroot["fnickname"].asString();

								/* 나의 회원정보를 불러옴 */
								DML = DML_Select(4, "user_num", "user_info", "user_nickname", nickname.c_str());
								PGresult *resmyusernum = PQexec(pCon, DML.c_str()); // DML SEND;
								int myusernum = stoi(PQgetvalue(resmyusernum, 0, 0));
								/*친구의 회원정보를 불러옴 */
								DML = DML_Select(4, "user_num", "user_info", "user_nickname", fnickname.c_str());
								PGresult *resfriendusernum = PQexec(pCon, DML.c_str()); // DML SEND;

								/* 해당 친구 닉네임이 유저 목록에 있는지 확인 */
								if (PQntuples(resfriendusernum) > 0)
								{
									int friendusernum = stoi(PQgetvalue(resfriendusernum, 0, 0));
									/* 나의 친구정보를 불러옴 */
									DML = "select friends_user_num from friends_info where user_num = " + to_string(myusernum) + ";";
									PGresult *resfriendlist = PQexec(pCon, DML.c_str()); // DML SEND;

									for (i = 0; i < PQntuples(resfriendlist); i++)
									{
										getfriendnum = stoi(PQgetvalue(resfriendlist, i, 0));
										if (getfriendnum == friendusernum)
											friendcnt++;
									}

									/* 이미 친구로 등록되어 있는경우 */
									if (friendcnt > 0)
									{
										sendroot["action"] = "addfriend";
										sendroot["result"] = "false";
										sendroot["msg"] = "이미 친구로 등록되어 있습니다";
									}

									/* 친구로 등록되어 있지 않은 경우 */
									else
									{
										DML = "insert into friends_info values(" + to_string(myusernum) + "," + to_string(friendusernum) + ")" + ";";
										PGresult *resfriendadd = PQexec(pCon, DML.c_str()); // DML SEND;

										sendroot["action"] = "addfriend";
										sendroot["result"] = "true";
										sendroot["msg"] = "친구추가가 완료되었습니다";
									}
								}

								/* 친구 닉네임이 유저 목록에 없는 경우 */
								else if (PQntuples(resfriendusernum) == 0)
								{
									sendroot["action"] = "addfriend";
									sendroot["result"] = "false";
									sendroot["msg"] = "잘못된 닉네임입니다";
								}

								data.msg.clear();
								data.msg = writer.write(sendroot);
								cout << writer.write(sendroot) << endl;
								data.size = data.msg.size();

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_changemyinfo_error\n"
										 << endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_changemyinfo_error\n"
											 << endl;
									else
										cout << "Send Success: "
											 << "(" << c[index].clnt_sock << ")" << endl;
								}

							} /* end addfriend */

							else if (action == "deletefriends")
							{
								int getfriendnum;
								int friendcnt = 0;
								string nickname = recvroot["nickname"].asString();
								string fnickname = recvroot["fnickname"].asString();

								/* 1. 나의 회원정보를 불러옴 */
								DML = DML_Select(4,"user_num","user_info","user_nickname",nickname.c_str());
								PGresult* resmyusernum = PQexec(pCon, DML.c_str());
								int myusernum = stoi(PQgetvalue(resmyusernum, 0, 0));
								/*2. 친구의 회원정보를 불러옴 */
								DML = DML_Select(4, "user_num", "user_info", "user_nickname", fnickname.c_str());
								PGresult* resfriendusernum = PQexec(pCon, DML.c_str());
								int friendusernum = stoi(PQgetvalue(resfriendusernum, 0, 0));

								/* 3. 해당 친구를 삭제 */
								DML = "delete from friends_info where user_num = " + to_string(myusernum) + " and "
								+ "friends_user_num = " + to_string(friendusernum) + ";";
								PGresult* resfrienddelete = PQexec(pCon, DML.c_str());

								if (PQresultStatus(resfrienddelete) == PGRES_COMMAND_OK)
								{
									sendroot["action"] = "deletefriends";
									sendroot["result"] = "true";
									sendroot["msg"] = "친구삭제가 완료되었습니다";
								}

								else
								{
									sendroot["action"] = "deletefriends";
									sendroot["result"] = "false";
									sendroot["msg"] = "친구삭제를 실패하였습니다";
								}

								data.msg.clear();
								data.msg = writer.write(sendroot);
								cout << writer.write(sendroot) << endl;
								data.size = data.msg.size();

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_deletefriends_error\n" <<endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_deletefriends_error\n" << endl;
									else
										cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
								}

							} /* end deletefriends */

							else if (action == "friendslist")
							{
								int getfriendusernum, getfrienduserstate;
								string getfriendnickname;
								string nickname = recvroot["nickname"].asString();
								/* 1. 나의 회원정보 찾기 */
								DML = DML_Select(4,"user_num", "user_info","user_nickname",nickname.c_str());
								PGresult* resmyusernum = PQexec(pCon, DML.c_str());
								int myusernum = stoi(PQgetvalue(resmyusernum, 0, 0));
								/* 2. 나의 친구목록 찾기 */
								DML = "select friends_user_num from friends_info where user_num = " + to_string(myusernum) + ";";
								PGresult* resfriendusernum = PQexec(pCon, DML.c_str());
								int cntfrienduser = PQntuples(resfriendusernum);

								Json::Value friendslist;
								Json::Value friends[cntfrienduser];

								for (int i = 0; i < cntfrienduser; i++)
								{
									getfriendusernum = stoi(PQgetvalue(resfriendusernum, i, 0));
									DML = "select user_nickname,user_state from user_info where user_num = " + to_string(getfriendusernum) + ";";
									PGresult* resselect = PQexec(pCon, DML.c_str());
									getfriendnickname = PQgetvalue(resselect, 0, 0);
									getfrienduserstate = stoi(PQgetvalue(resselect, 0, 1));
									friends[i]["nickname"] = getfriendnickname;
									if (getfrienduserstate % 2 == 0)
										friends[i]["fstate"] = "offline";
									else
										friends[i]["fstate"] = "oneline";
									friendslist.append(friends[i]);
								}

								sendroot["action"] = "friendslist";
								sendroot["result"] = "true";
								sendroot["friendslist"] = friendslist;

								data.msg.clear();
								data.msg = writer.write(sendroot);
								cout << writer.write(sendroot) << endl;
								data.size = data.msg.size();

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_friendslist_error\n" <<endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_friendslist_error\n" << endl;
									else
										cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
								}
							}	/* end friendslist */

							else if (action == "changemyinfo")
							{
								string id = recvroot["id"].asString();
								string nickname = recvroot["nickname"].asString();
								string phone = recvroot["phone"].asString();
								string pw = recvroot["pw"].asString();

								/* 본인 닉네임 */
								DML = DML_Select(6,"user_nickname","user_info","user_id",id.c_str(),"user_pw",pw.c_str());
								PGresult* resmynickname = PQexec(pCon, DML.c_str()); //DML SEND;
								string dbmynickname = PQgetvalue(resmynickname, 0, 0);

							    /* 전체 닉네임 중복 검사 */
								DML = DML_Select(4,"*","user_info","user_nickname",nickname.c_str());
								PGresult* restotalnickname = PQexec(pCon, DML.c_str()); //DML SEND;

								/* 비밀번호 일치 검사 */
								DML = DML_Select(4,"*","user_info","user_id",id.c_str(),"user_pw",pw.c_str());
								PGresult* respwcheck = PQexec(pCon, DML.c_str()); //DML SEND;

								/* 1. 이전에 입력한 닉네임과 일치하는 경우 */
								if (dbmynickname == nickname)
								{
									DML = DML_Update(9,"user_info","user_nickname",nickname.c_str(),
											"user_phone",phone.c_str(), "user_id",id.c_str(),"user_pw",pw.c_str());

									sendroot["action"] = "changemyinfo";
									sendroot["result"] = "true";
									sendroot["msg"] = "회원정보 수정이 완료되었습니다";
								}

								/* 2. 전체 닉네임을 조회했을 때 중복인 경우 */
								else if ((dbmynickname != nickname) && PQntuples(restotalnickname) > 0 && PQntuples(respwcheck) > 0)
								{
									sendroot["action"] = "changemyinfo";
									sendroot["result"] = "false";
									sendroot["msg"] = "중복된 닉네임입니다";
								}

								/* 3. 전체 닉네임을 조회했을 때 중복이 아닌 경우 */
								else if ((dbmynickname != nickname) && PQntuples(restotalnickname) == 0 && PQntuples(respwcheck) > 0)
								{
									DML = DML_Update(9,"user_info","user_nickname",nickname.c_str(),
											"user_phone",phone.c_str(), "user_id",id.c_str(),"user_pw",pw.c_str());

									sendroot["action"] = "changemyinfo";
									sendroot["result"] = "true";
									sendroot["msg"] = "회원정보 수정이 완료되었습니다";
								}

								/* 4. 비밀번호가 일치하지 않는 경우 */
								else if (PQntuples(respwcheck) == 0)
								{
									sendroot["action"] = "changemyinfo";
									sendroot["result"] = "false";
									sendroot["msg"] = "비밀번호가 일치하지 않습니다";
								}

								data.msg.clear();
								data.msg = writer.write(sendroot);
								cout << writer.write(sendroot) << endl;
								data.size = data.msg.size();

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_changemyinfo_error\n" <<endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_changemyinfo_error\n" << endl;
									else
										cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
								}
							} /* end changemyinfo */

							else if (action == "deleteaccount")
							{
								string nickname = recvroot["nickname"].asString();

								/* 1. 해당 회원의 회원번호 찾기 */
								DML = DML_Select(4,"user_num","user_info","user_nickname",nickname.c_str());
								PGresult* resusernum = PQexec(pCon, DML.c_str()); //DML SEND;
								int myusernum = stoi(PQgetvalue(resusernum, 0, 0));

								/* 2. 해당 회원의 친구정보 삭제 */
								DML = "delete from friends_info where friends_num = " + to_string(myusernum) + ";";
								PGresult* resmydeletefriend = PQexec(pCon, DML.c_str()); //DML SEND;

								/* 3. 해당 회원을 친구로 등록하고 있는 회원에게서 해당 회원을 삭제 */
								DML = "select user_num from friends_info where friends_user_num = " + to_string(myusernum) + ";";
								PGresult* resfriendusernum = PQexec(pCon, DML.c_str()); //DML SEND;
								int friendusernum = stoi(PQgetvalue(resfriendusernum, 0, 0));
								DML = "delete from friends_info where user_num = " + to_string(friendusernum) + " and " + "friends_user_num = " + to_string(myusernum) + ";";
								PGresult* resfrienddelete = PQexec(pCon, DML.c_str()); //DML SEND;

								/* 4. 회원정보 삭제 */
								DML = "delete from user_info where user_num = " + to_string(myusernum) + ";";
								PGresult* resuserdelete = PQexec(pCon, DML.c_str()); //DML SEND;

								/* 해당 회원의 친구정보 삭제 + 친구 삭제 + 회원정보 삭제 */
								if (PQresultStatus(resmydeletefriend) == PGRES_COMMAND_OK && PQresultStatus(resfrienddelete) == PGRES_COMMAND_OK &&
										PQresultStatus(resuserdelete) == PGRES_COMMAND_OK)
								{
									sendroot["action"] = "deleteaccount";
									sendroot["result"] = "true";
									sendroot["msg"] = "회원탈퇴가 완료되었습니다";
								}

								/* Json Data Send */
								data.msg.clear();
								data.msg = writer.write(sendroot);
								data.size = data.msg.size();

								if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
									cout << "ret_HeadWrite_changemyinfo_error\n" <<endl;

								else // HeadWrite Successful
								{
									if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
										cout << "ret_BodyWrite_changemyinfo_error\n" << endl;
									else
										cout << "Send Success: " <<"("<< c[index].clnt_sock <<")"  << endl;
								}
							} /* end deleteaccount */




						}
					}
				}

				SendData(ssl, data);
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
	int serv_sock;				  /* socket descriptor             */

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	int option;
	option = 1;

	/* Map port number (char string) to port number (int) */
	if ((serv_addr.sin_port = htons((unsigned short)atoi(portnum))) == 0)
		printf("can't get \"%s\" port number\n", portnum);

	/* Allocate a socket */
	serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serv_sock < 0)
		printf("can't create socket: %s\n", strerror(errno));

	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	/* Bind the socket */
	if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "can't bind to %s port: %s; Trying other port\n",
				portnum, strerror(errno));
		serv_addr.sin_port = htons(0); /* request a port number to be allocated
								 by bind */
	}
	else
	{
		socklen_t socklen = sizeof(serv_addr);
	}

	if (listen(serv_sock, qlen) < 0)
		printf("can't listen on %s port: %s\n", portnum, strerror(errno));
	return serv_sock;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////////DML////////////////////////////////////
////////////////////////////////////////////////////////////////////////

string DML_Insert(string table, ...)
{
	string DML = "insert into " + table + " values (";
	va_list args;
	va_start(args, table);
	char *gData;

	if (table == "user_info")
	{
		DML += "nextval('sq_user_num'),'"; // user_num : sequence
		for (int i = 0; i < 6; i++)
		{
			gData = va_arg(args, char *);
			if (i != 5)
			{
				DML += gData;
				DML += "','";
			}
			else
				DML += gData;
		}
		DML += "',0,0);";
	}

	else if (table == "friends_info")
	{
		DML += "'";
		for (int i = 0; i < 2; i++)
		{
			gData = va_arg(args, char *);
			if (i != 1)
			{
				DML += gData;
				DML += "','";
			}
			else
				DML += gData;
		}
		DML += "');";
	}

	else if (table == "room_info")
	{
		DML += "nextval('room_num'),"; // room_num : sequence
		for (int i = 0; i < 5; i++)
		{
			gData = va_arg(args, char *);
			if (i < 3)
			{
				DML += gData;
				DML += ",";
			}
			else if (i == 3)
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

	for (int i = 0; i < args; i++)
	{
		gData = va_arg(ap, char *);
		if (i == 0)
			DML += gData;
		else if (i == 1)
		{
			DML += " from ";
			DML += gData;
			DML += " where ";
		}
		else
		{
			if (i % 2 == 0)
			{
				DML += gData;
				DML += " = '";
			}
			else if (i == args - 1)
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

	for (int i = 0; i < args; i++)
	{
		gData = va_arg(ap, char *);
		if (i == 0)
		{
			DML += gData;
			DML += " where ";
		}
		else
		{
			if (i % 2 != 0)
			{
				DML += gData;
				DML += " = '";
			}
			else if (i == args - 1)
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

	for (int i = 0; i < args; i++)
	{
		gData = va_arg(ap, char *);
		if (i == 0)
		{
			DML += gData;
			DML += " set ";
		}
		else if (i == 1)
		{
			DML += gData;
			DML += " = '";
		}
		else if (i == 2)
		{
			DML += gData;
			DML += "' where ";
		}
		else
		{
			if (i % 2 == 1)
			{
				DML += gData;
				DML += " = '";
			}
			else if (i == args - 1)
			{
				DML += gData;
				DML += "';";
			}
		}
	}
	va_end(ap);
	return DML;
}

void SendData(SSL *ssl, Data data)
{
	int ret_HeadWrite = 0;
	if (ret_HeadWrite = SSL_write(ssl, &data.size, sizeof(int)) <= 0)
		cout << "ret_HeadWrite_error\n"
			 << endl;

	else // HeadWrite Successful
	{
		int ret_BodyWrite = 0;
		if (ret_BodyWrite = SSL_write(ssl, &data.msg[0], data.size) <= 0)
			cout << "ret_BodyWrite_error\n"
				 << endl;
	}
}