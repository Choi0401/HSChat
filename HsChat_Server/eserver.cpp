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

#include <mutex>
#include <thread>
#include <queue>

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

using std::thread;

template <typename T>
class TsQueue
{
	std::queue<T> queue_;
	mutable std::mutex mutex_;

public:
	TsQueue() = default;
	TsQueue(const TsQueue<T> &) = delete;
	TsQueue &operator=(const TsQueue<T> &) = delete;

	TsQueue(TsQueue<T> &&other)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_ = std::move(other.queue_);
	}

	virtual ~TsQueue() {}

	unsigned long size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}

	bool empty() const
	{
		return queue_.empty();
	}

	T pop()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty())
		{
			return {};
		}
		T tmp = queue_.front();
		queue_.pop();
		return tmp;
	}

	void push(const T &item)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(item);
	}
};

void rcv(void *p);

typedef struct Trcv
{
	int index;
	int fd_arr;
	int fd;
	SSL *ssl;
	TsQueue<string> Queue;
	fd_set fd_cpy_reads;
	Data data;
} Trcv;

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
	int state;
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

	// ???????????? ?????? ????????? ????????? ?????? ?????????
	Data data;

	//????????? ?????????
	Trcv trcv;

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
		c[i].current_room_num = 0; // ?????????
		c[i].ismaster = false;
		c[i].clientssl = NULL;
		c[i].usernum = 0;
		c[i].state = 0;
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

	int cntroom = 0; // ????????? ??????
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
				/////////////////////////////////////////////////////////////////////////////
				/////////////////////////////////TREAD///////////////////////////////////////
				trcv.index = index;
				trcv.fd_arr = fd_arr[index];
				trcv.fd = fd;
				trcv.ssl = ssl;
				// trcv.Queue = Queue;
				trcv.fd_cpy_reads = fd_cpy_reads;
				trcv.data = data;

				thread _thread(rcv, &trcv);
				_thread.join();

				data.msg = trcv.Queue.pop();

				bool parseSuccessful = reader.parse(data.msg, recvroot);
				if (parseSuccessful == false)
				{
					// std::cout << "Failed to parse configuration\n" << reader.getFormatedErrorMessages();

					// return -1;
				}

				else
				{
					sendroot.clear();
					string action = recvroot["action"].asString();
					if (action == "signup")
					{
						string id = recvroot["id"].asString();
						string pw = recvroot["pw"].asString();
						string nickname = recvroot["nickname"].asString();
						string name = recvroot["name"].asString();
						string birth = recvroot["birth"].asString();
						string phone = recvroot["phone"].asString();

						// 1. ????????? ?????? ??????
						DML = DML_Select(4, "*", "user_info", "user_id", id.c_str());
						PGresult *resID = PQexec(pCon, DML.c_str()); // DML SEND;

						if (PQresultStatus(resID) == PGRES_TUPLES_OK)
						{
							// 2. ????????? ?????? ??????
							DML = DML_Select(4, "*", "user_info", "user_nickname", nickname.c_str());
							PGresult *resNickname = PQexec(pCon, DML.c_str()); // DML SEND;
							if (PQresultStatus(resNickname) == PGRES_TUPLES_OK)
							{
								// 3. ???????????? ?????? ??????
								DML = DML_Select(4, "*", "user_info", "user_phone", phone.c_str());
								PGresult *resPhone = PQexec(pCon, DML.c_str());

								//????????? ???????????? ?????? ??????
								if (PQntuples(resID) == 0 && PQntuples(resNickname) == 0 && PQntuples(resPhone) == 0)
								{
									DML = "insert into user_info values(nextval('sq_user_num'),'" + name + "','" + birth + "','" + phone + "','" + id + "','" + nickname + "','" + pw + "',0,0);";
									PGresult *resInsert = PQexec(pCon, DML.c_str()); // DML SEND

									if (PQresultStatus(resInsert) == PGRES_COMMAND_OK)
									{
										sendroot["action"] = "signup";
										sendroot["result"] = "true";
										sendroot["msg"] = nickname + "??? ???????????????";

										DML = DML_Select(4, "user_num", "user_info", "user_nickname", nickname.c_str());
										PGresult *resSelect = PQexec(pCon, DML.c_str()); // DML SEND
										if (PQntuples(resSelect) == 1)
										{
											c[index].usernum = atoi(PQgetvalue(resSelect, 0, 0));
										}
									}
									else
									{
										cout << "Insert ERROR" << endl;
										sendroot["result"] = "false";
										sendroot["msg"] = "??????????????? ?????????????????????";
										cout << PQerrorMessage(pCon) << endl;
									}
								}
								//?????? ????????? ???????????? ??????
								else if (PQntuples(resID) > 0 || PQntuples(resNickname) > 0 || PQntuples(resPhone) > 0)
								{
									sendroot["action"] = "signup";
									sendroot["result"] = "false";
									if (PQntuples(resID) > 0)
										sendroot["msg"] = "?????? ???????????? ?????? ???????????????";
									else if (PQntuples(resNickname) > 0)
										sendroot["msg"] = "?????? ???????????? ?????? ??????????????????";
									else if (PQntuples(resPhone) > 0)
										sendroot["msg"] = "?????? ???????????? ?????? ?????????????????????";
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
						string HIS_DML;
						string nickname, signin_ok;

						// ?????????, ???????????? ??????
						DML = DML_Select(6, "user_nickname", "user_info", "user_id", id.c_str(), "user_pw", pw.c_str());
						PGresult *rescheck = PQexec(pCon, DML.c_str()); // DML SEND;

						if (PQresultStatus(rescheck) == PGRES_TUPLES_OK)
						{

							if (PQntuples(rescheck) == 0)
							{
								sendroot["action"] = "signin";
								sendroot["result"] = "false";
								sendroot["msg"] = "???????????? ??????????????? ??????????????????";
								signin_ok = "false";
							}
							else if (PQntuples(rescheck) == 1)
							{
								// ????????? ????????? ??????
								DML = "select user_state from user_info where user_id = '" + id + "';";
								PGresult *reschecklogin = PQexec(pCon, DML.c_str()); // DML SEND;
								if (PQntuples(reschecklogin) == 1)
								{
									sendroot["action"] = "signin";
									int state = atoi(PQgetvalue(reschecklogin, 0, 0));
									if (state == 0)
									{
										nickname = PQgetvalue(rescheck, 0, 0);
										c[index].nickname = nickname;
										c[index].id = id;

										sendroot["result"] = "true";
										sendroot["nickname"] = nickname;
										sendroot["id"] = id;
										sendroot["msg"] = nickname + "??? ???????????????";

										// ????????? ?????? ??????????????? ????????????
										DML = "update user_info set user_state = " + to_string(1) + " where user_nickname = '" + nickname + "';";
										PGresult *resUpdate = PQexec(pCon, DML.c_str()); // DML SEND;
										PQclear(resUpdate);
										signin_ok = "true";

										//
										DML = DML_Select(4, "user_num", "user_info", "user_nickname", nickname.c_str());
										PGresult *resSelectUser = PQexec(pCon, DML.c_str()); // DML SEND
										if (PQntuples(resSelectUser) == 1)
										{
											c[index].usernum = atoi(PQgetvalue(resSelectUser, 0, 0));
											c[index].current_room_num = 0;
											c[index].ismaster = false;
											c[index].state = 1;
										}
									}
									else if (state == 1)
									{
										sendroot["result"] = "false";
										sendroot["msg"] = "?????? ??????????????????";
										signin_ok = "false";
									}
								}
							}
							time_t now = time(0);
							string ttt;

							tm *ltm = localtime(&now);

							ttt += to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec);

							HIS_DML = "insert into login_his values(nextval('sq_login')," + to_string(c[index].usernum) + ", '" + ttt + "', '" + c[index].clnt_ip + "','" + signin_ok + "');";
							cout << HIS_DML << endl;
							PGresult *SigninHis = PQexec(pCon, HIS_DML.c_str());

							if (PQresultStatus(SigninHis) != PGRES_COMMAND_OK)
							{
								cout << "LOGIN_HIS ERROR" << endl;
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
											cout << "????????? ?????? : " << room[cntroom].m_roomnum << endl;
											room[cntroom].m_master = master;
											cout << "????????? ?????? : " << room[cntroom].m_master << endl;
											room[cntroom].m_name = roomname;
											cout << "????????? ?????? : " << room[cntroom].m_name << endl;
											room[cntroom].m_type = roomtype;
											cout << "????????? ??????: " << room[cntroom].m_type << endl;
											room[cntroom].m_usernum = 1;
											cout << "????????? ?????? ?????? : " << room[cntroom].m_usernum << endl;
											room[cntroom].m_maxnum = maxnum;
											cout << "????????? ??? ?????? : " << room[cntroom].m_maxnum << endl;
											room[cntroom].m_client.push_back(c[index]);
											cout << "????????? ?????? IP: " << room[cntroom].m_client[room[cntroom].m_usernum - 1].clnt_ip << endl;
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
												c[index].current_room_num = room[cntroom].m_roomnum;
												c[index].ismaster = true;
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
									sendroot["msg"] = "??? ???????????? ?????????????????????";
								}
								PQclear(resSelect);
							}
							else if (PQntuples(resSelect) == 0)
							{
								cout << "Cannot find user_num" << endl;
								sendroot["result"] = "false";
								sendroot["msg"] = "??? ???????????? ?????????????????????";
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
						int getfriendusernum, getfrienduserstate;
						string getfriendnickname;

						if (PQresultStatus(resSelect) == PGRES_TUPLES_OK)
						{

							int cnttuples = PQntuples(resSelect);
							if (cnttuples >= 0)
							{
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

								// data.msg.clear();
								// data.msg = writer.write(sendroot);
								// data.size = data.msg.size();
							}
						}
						else
						{
							cout << PQerrorMessage(pCon) << endl;
						}

						// friends_list//
						DML = "select friends_user_num from friends_info where user_num = " + to_string(c[index].usernum) + ";";
						PGresult *select_user_num = PQexec(pCon, DML.c_str());
						DML.clear();

						if (PQresultStatus(select_user_num) != PGRES_TUPLES_OK)
						{
							cout << "Friends_list ERROR" << endl;
							return 0;
						}

						int num_friends = PQntuples(select_user_num);
						if (num_friends <= 0)
						{
							cout << "no friends_num" << endl;
						}

						else
						{
							Json::Value friendslist;
							Json::Value friends[num_friends];

							for (int i = 0; i < num_friends; i++)
							{
								getfriendusernum = stoi(PQgetvalue(select_user_num, i, 0));
								DML = "select user_nickname,user_state from user_info where user_num = " + to_string(getfriendusernum) + ";";
								PGresult *resselect = PQexec(pCon, DML.c_str());
								if (PQresultStatus(resselect) != PGRES_TUPLES_OK)
									cout << "select friendlist error" << endl;

								getfriendnickname = PQgetvalue(resselect, 0, 0);
								getfrienduserstate = stoi(PQgetvalue(resselect, 0, 1));
								friends[i]["nickname"] = getfriendnickname;
								if (getfrienduserstate % 2 == 0)
									continue;
								else
									friends[i]["fstate"] = "online";
								friendslist.append(friends[i]);
							}

							sendroot["friendslist"] = friendslist;
						}

						data.msg.clear();
						data.msg = writer.write(sendroot);
						data.size = data.msg.size();
						PQclear(resSelect);
					}
					else if (action == "quitroom")
					{
						string nickname = recvroot["nickname"].asString();
						// Get user_num
						DML = DML_Select(4, "user_num", "user_info", "user_nickname", nickname.c_str());
						PGresult *resSelectuser = PQexec(pCon, DML.c_str());

						if (PQresultStatus(resSelectuser) == PGRES_TUPLES_OK)
						{
							if (PQntuples(resSelectuser) == 1)
							{
								int user_num = atoi(PQgetvalue(resSelectuser, 0, 0));
								// Get room info
								DML = "select * from room_info;";
								PGresult *resSelectroom = PQexec(pCon, DML.c_str());

								if (PQresultStatus(resSelectroom) == PGRES_TUPLES_OK)
								{

									int cnttuples = PQntuples(resSelectroom);

									if (cnttuples > 0)
									{
										c[index].ismaster = false;
										int roomnum, currentusernum, masterusernum;
										for (int i = 0; i < cnttuples; i++) // Get tables data
										{
											roomnum = stoi(PQgetvalue(resSelectroom, i, 0));
											masterusernum = stoi(PQgetvalue(resSelectroom, i, 1));
											currentusernum = stoi(PQgetvalue(resSelectroom, i, 2));
										}

										// ?????? ?????? ?????? ??????
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
											// ????????? ????????? ?????? ????????????
											DML = "update user_info set current_room_num = " + to_string(0) + " where user_num = " + to_string(user_num) + ";";
											PGresult *resUpdate = PQexec(pCon, DML.c_str());

											if (PQresultStatus(resUpdate) == PGRES_COMMAND_OK)
											{
												// ????????? DB?????? ??????
												DML = "delete from room_info where room_num = " + to_string(roomnum) + ";";
												PGresult *resDelete = PQexec(pCon, DML.c_str());
												cout << PQresultStatus(resDelete) << endl;
											}
											else
											{
												cout << PQerrorMessage(pCon) << endl;
											}
											c[index].current_room_num = 0;
											PQclear(resUpdate);
											continue;
										}
										else if (currentusernum > 1)
										{
											int roomindex = 0, clientindex = 0;
											// ??? ?????????
											for (int i = 0; i < room.size(); i++)
											{
												if (room[i].m_roomnum == roomnum)
												{
													roomindex = i;
													room[i].m_usernum--;
													break;
												}
											}
											// ??????????????? ?????????
											for (int j = 0; j < room[roomindex].m_client.size(); j++)
											{
												if (room[roomindex].m_client[j].nickname == nickname)
													clientindex = j;
											}
											// ??????????????? ????????? ?????? ?????? ??????
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
														cout << "room[" << roomindex << "].m_client[" << i << "]usernum = " << room[roomindex].m_client[i].usernum << endl;

														if (room[roomindex].m_client[i].usernum == nextusernum)
														{
															room[roomindex].m_client[i].ismaster = true;
															room[roomindex].m_master = room[roomindex].m_client[i].nickname;
															cout << "master = " << room[roomindex].m_master;
															sendroot["nextmaster"] = room[roomindex].m_master;
															break;
														}
													}
													room[roomindex].m_client[clientindex].ismaster = false;
													// ????????? ????????? ?????? ????????????
													DML = "update user_info set current_room_num = " + to_string(0) + " where user_num = " + to_string(user_num) + ";";
													PGresult *resUpdateuser = PQexec(pCon, DML.c_str());
													// ????????? ?????? ????????? ????????????
													DML = "update room_info set room_num_user = " + to_string(room[roomindex].m_usernum) + " where room_num = " + to_string(roomnum) + ";";
													resUpdateroom = PQexec(pCon, DML.c_str());
													currentusernum--;
													room[roomindex].m_client.erase(room[roomindex].m_client.begin() + clientindex);
												}
												else
													cout << PQerrorMessage(pCon) << endl;
												PQclear(resselectuser);
												sendroot["ismaster"] = "true";
											}
											else
											{
												// ????????? ????????? ?????? ????????????
												DML = "update user_info set current_room_num = " + to_string(0) + " where user_num = " + to_string(user_num) + ";";
												PGresult *resUpdateuser = PQexec(pCon, DML.c_str());
												// ????????? ?????? ????????? ????????????
												DML = "update room_info set room_num_user = " + to_string(room[roomindex].m_usernum) + " where room_num = " + to_string(roomnum) + ";";
												PGresult *resUpdateroom = PQexec(pCon, DML.c_str());
												room[roomindex].m_client.erase(room[roomindex].m_client.begin() + clientindex);
												currentusernum--;
												sendroot["ismaster"] = "false";
											}
											c[index].current_room_num = 0;
											/* ?????? ????????? ???????????? ????????? */
											// sendroot.clear();
											sendroot["action"] = "updateuserlist";
											sendroot["inout"] = "out";
											sendroot["nickname"] = nickname;
											string sendmsg = "[??????]" + nickname + "?????? ??????????????? ?????????????????????.\r\n";
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
						PQclear(resSelectuser);
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
							sendroot["msg"] = "???????????? ????????? ??? ????????????";

							data.msg.clear();
							data.msg = writer.write(sendroot);
							data.size = data.msg.size();
						}

						DML = "select room_num_user, room_max_user from room_info where room_num = " + to_string(roomnum) + ";";
						resSelecteNum = PQexec(pCon, DML.c_str());
						// string room_num_user = PQgetvalue(resSelecteNum, 0, 0);
						// string room_max_user = PQgetvalue(resSelecteNum, 0, 1);
						if (PQntuples(resSelecteNum) > 0)
						{
							int room_num_user = atoi(PQgetvalue(resSelecteNum, 0, 0));
							int room_max_user = atoi(PQgetvalue(resSelecteNum, 0, 1));

							if (room_num_user == room_max_user)
							{
								sendroot["action"] = "enterroom";
								sendroot["result"] = "false";
								sendroot["msg"] = "???????????? ???????????????.";

								data.msg.clear();
								data.msg = writer.write(sendroot);
								data.size = data.msg.size();
							}
							else
							{
								// ????????? ?????? ?????? ??? ????????????
								DML = "update room_info set room_num_user = " + to_string(++room[roomindex].m_usernum) + " where room_num = " + to_string(roomnum) + ";";
								PGresult *resUpdateNum = PQexec(pCon, DML.c_str()); // DML SEND;
								room[roomindex].m_client.push_back(c[index]);

								// ??????????????? ?????? ????????? ?????? ????????????
								DML = "update user_info set current_room_num = " + to_string(roomnum) + " where user_nickname = '" + nickname + "';";
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
								sendroot["msg"] = "???????????? ?????? ?????? ???????????????.\r\n\r\n";
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

								/* ?????? ????????? ???????????? ????????? */
								sendroot.clear();
								sendroot["action"] = "updateuserlist";
								sendroot["inout"] = "in";
								sendroot["nickname"] = nickname;
								string sendmsg = "[??????]" + nickname + "?????? ???????????? ?????????????????????.\r\n";
								sendroot["msg"] = sendmsg;

								data.msg.clear();
								data.msg = writer.write(sendroot);
								cout << data.msg << endl;
								data.size = data.msg.size();

								int cnt_client = room[roomindex].m_client.size();
								for (int i = 0; i < cnt_client; i++)
								{
									if (room[roomindex].m_client[i].nickname != nickname)
									{
										SendData(room[roomindex].m_client[i].clientssl, data);
									}
								}
								c[index].current_room_num = roomnum;
								continue;
							}
						}
						else
						{
							sendroot["action"] = "enterroom";
							sendroot["result"] = "false";
							sendroot["msg"] = "???????????? ????????? ??? ????????????";

							data.msg.clear();
							data.msg = writer.write(sendroot);
							data.size = data.msg.size();
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
						timer = time(NULL);	   // 1970??? 1??? 1??? 0??? 0??? 0????????? ???????????? ??????????????? ???
						t = localtime(&timer); // ???????????? ?????? ???????????? ??????
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
						if (PQntuples(resSearchID) > 0)					   // id??? ?????? ??????
						{
							string id = PQgetvalue(resSearchID, 0, 0);
							sendroot["action"] = "searchid";
							sendroot["result"] = "true";
							sendroot["msg"] = "????????????" + id + " ?????????";

							/* Json Data Send */
							data.msg.clear();
							data.msg = writer.write(sendroot);
							data.size = data.msg.size();
						}

						else if (PQntuples(resSearchID) == 0) // id??? ?????? ?????? ??????
						{
							sendroot["action"] = "searchid";
							sendroot["result"] = "false";
							sendroot["msg"] = "???????????? ????????? ???????????? ????????????";

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

						if (PQntuples(resSearchPW) > 0) // pw??? ?????? ??????
						{
							string pw = PQgetvalue(resSearchPW, 0, 0);
							sendroot["action"] = "searchpw";
							sendroot["result"] = "true";
							sendroot["msg"] = "???????????????" + pw + " ?????????";

							/* Json Data Send */
							data.msg.clear();
							data.msg = writer.write(sendroot);
							data.size = data.msg.size();
						}

						else if (PQntuples(resSearchPW) == 0) // id??? ?????? ?????? ??????
						{
							sendroot["action"] = "searchpw";
							sendroot["result"] = "false";
							sendroot["msg"] = "???????????? ????????? ???????????? ????????????";

							/* Json Data Send */
							data.msg.clear();
							data.msg = writer.write(sendroot);
							data.size = data.msg.size();
						}

					} /* end searchpw */

					else if (action == "setnewpw")
					{
						string id = recvroot["id"].asString();
						string pw = recvroot["pw"].asString();

						DML = "update user_info set user_pw = '" + pw + "' where user_id = " + "'" + id + "'" + ";";
						PGresult *resUpdate = PQexec(pCon, DML.c_str());
						// cout << DML << endl;

						if (PQresultStatus(resUpdate) == PGRES_COMMAND_OK)
						{
							sendroot["action"] = "setnewpw";
							sendroot["result"] = "true";
							sendroot["msg"] = "???????????? ???????????? ?????????????????????";
							data.msg.clear();
							data.msg = writer.write(sendroot);
							data.size = data.msg.size();
						}
						else
						{
							cout << PQerrorMessage(pCon) << endl;
						}
					}

					else if (action == "showmyinfo")
					{
						const char *gdata;
						string nickname = recvroot["nickname"].asString();
						string id = c[index].id;
						DML = "select user_name, user_birth from user_info where user_id = '" + id + "';";

						PGresult *resshowmyinfo = PQexec(pCon, DML.c_str()); // DML SEND;

						string name = PQgetvalue(resshowmyinfo, 0, 0);
						string birth = PQgetvalue(resshowmyinfo, 0, 1);

						if (PQntuples(resshowmyinfo) > 0) // ???????????? ???????????? ?????? ??????(????????? ??????)
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

					} /* end showmyinfo */

					else if (action == "addfriend")
					{
						int getfriendnum;
						int friendcnt = 0;
						string nickname = recvroot["nickname"].asString();
						string fnickname = recvroot["fnickname"].asString();
						string id = c[index].id;

						/* ?????? ??????????????? ????????? */
						DML = DML_Select(4, "user_num", "user_info", "user_id", id.c_str());
						PGresult *resmyusernum = PQexec(pCon, DML.c_str()); // DML SEND;
						int myusernum = stoi(PQgetvalue(resmyusernum, 0, 0));
						/*????????? ??????????????? ????????? */
						DML = DML_Select(4, "user_num", "user_info", "user_nickname", fnickname.c_str());
						PGresult *resfriendusernum = PQexec(pCon, DML.c_str()); // DML SEND;

						/* ?????? ?????? ???????????? ?????? ????????? ????????? ?????? */
						if (PQntuples(resfriendusernum) > 0)
						{
							int friendusernum = stoi(PQgetvalue(resfriendusernum, 0, 0));
							/* ?????? ??????????????? ????????? */
							DML = "select friends_user_num from friends_info where user_num = " + to_string(myusernum) + ";";
							PGresult *resfriendlist = PQexec(pCon, DML.c_str()); // DML SEND;

							for (i = 0; i < PQntuples(resfriendlist); i++)
							{
								getfriendnum = stoi(PQgetvalue(resfriendlist, i, 0));
								if (getfriendnum == friendusernum)
									friendcnt++;
							}

							/* ?????? ????????? ???????????? ???????????? */
							if (friendcnt > 0)
							{
								sendroot["action"] = "addfriend";
								sendroot["result"] = "false";
								sendroot["msg"] = "?????? ????????? ???????????? ????????????";
							}

							/* ????????? ???????????? ?????? ?????? ?????? */
							else
							{
								if (friendusernum != myusernum)
								{
									DML = "insert into friends_info values(" + to_string(myusernum) + "," + to_string(friendusernum) + ")" + ";";
									PGresult *resfriendadd = PQexec(pCon, DML.c_str()); // DML SEND;

									sendroot["action"] = "addfriend";
									sendroot["result"] = "true";
									sendroot["msg"] = "??????????????? ?????????????????????";
								}

								else
								{
									sendroot["action"] = "addfriend";
									sendroot["result"] = "false";
									sendroot["msg"] = "????????? ????????? ????????? ??? ????????????";
								}
							}
						}

						/* ?????? ???????????? ?????? ????????? ?????? ?????? */
						else if (PQntuples(resfriendusernum) == 0)
						{
							sendroot["action"] = "addfriend";
							sendroot["result"] = "false";
							sendroot["msg"] = "????????? ??????????????????";
						}

						data.msg.clear();
						data.msg = writer.write(sendroot);
						cout << writer.write(sendroot) << endl;
						data.size = data.msg.size();

					} /* end addfriend */

					else if (action == "deletefriends")
					{
						int getfriendnum;
						int friendcnt = 0;
						string nickname = recvroot["nickname"].asString();
						string fnickname = recvroot["fnickname"].asString();
						string id = c[index].id;

						/* 1. ?????? ??????????????? ????????? */
						DML = DML_Select(4, "user_num", "user_info", "user_id", id.c_str());
						PGresult *resmyusernum = PQexec(pCon, DML.c_str());
						int myusernum = stoi(PQgetvalue(resmyusernum, 0, 0));
						/*2. ????????? ??????????????? ????????? */
						DML = DML_Select(4, "user_num", "user_info", "user_nickname", fnickname.c_str());
						PGresult *resfriendusernum = PQexec(pCon, DML.c_str());
						int friendusernum = stoi(PQgetvalue(resfriendusernum, 0, 0));

						/* 3. ?????? ????????? ?????? */
						DML = "delete from friends_info where user_num = " + to_string(myusernum) + " and " + "friends_user_num = " + to_string(friendusernum) + ";";
						PGresult *resfrienddelete = PQexec(pCon, DML.c_str());

						if (PQresultStatus(resfrienddelete) == PGRES_COMMAND_OK)
						{
							sendroot["action"] = "deletefriends";
							sendroot["result"] = "true";
							sendroot["msg"] = "??????????????? ?????????????????????";
						}

						else
						{
							sendroot["action"] = "deletefriends";
							sendroot["result"] = "false";
							sendroot["msg"] = "??????????????? ?????????????????????";
						}

						data.msg.clear();
						data.msg = writer.write(sendroot);
						cout << writer.write(sendroot) << endl;
						data.size = data.msg.size();

					} /* end deletefriends */

					else if (action == "friendslist")
					{
						int getfriendusernum, getfrienduserstate;
						string getfriendnickname;
						string nickname = recvroot["nickname"].asString();
						string id = c[index].id;
						/* 1. ?????? ???????????? ?????? */
						DML = DML_Select(4, "user_num", "user_info", "user_id", id.c_str());
						PGresult *resmyusernum = PQexec(pCon, DML.c_str());
						int myusernum = stoi(PQgetvalue(resmyusernum, 0, 0));
						/* 2. ?????? ???????????? ?????? */
						DML = "select friends_user_num from friends_info where user_num = " + to_string(myusernum) + ";";
						PGresult *resfriendusernum = PQexec(pCon, DML.c_str());
						int cntfrienduser = PQntuples(resfriendusernum);

						Json::Value friendslist;
						Json::Value friends[cntfrienduser];

						for (int i = 0; i < cntfrienduser; i++)
						{
							getfriendusernum = stoi(PQgetvalue(resfriendusernum, i, 0));
							DML = "select user_nickname,user_state from user_info where user_num = " + to_string(getfriendusernum) + ";";
							PGresult *resselect = PQexec(pCon, DML.c_str());
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

					} /* end friendslist */

					else if (action == "changemyinfo")
					{

						string id = c[index].id;
						string nickname = recvroot["nickname"].asString();
						string phone = recvroot["phone"].asString();
						string pw = recvroot["pw"].asString();

						/* ?????? ????????? */
						DML = "select user_nickname, user_phone from user_info where user_id = '" + id + "';";

						PGresult *resselect = PQexec(pCon, DML.c_str()); // DML SEND;
						string dbmynickname = PQgetvalue(resselect, 0, 0);
						string dbmyphone = PQgetvalue(resselect, 0, 1);

						/* ?????? ????????? ?????? ?????? */
						DML = DML_Select(4, "*", "user_info", "user_nickname", nickname.c_str());
						PGresult *restotalnickname = PQexec(pCon, DML.c_str()); // DML SEND;

						/* ?????? ???????????? ?????? ?????? */
						DML = DML_Select(4, "*", "user_info", "user_phone", phone.c_str());
						PGresult *restotalphone = PQexec(pCon, DML.c_str()); // DML SEND;

						/* ???????????? ?????? ?????? */
						DML = DML_Select(6, "*", "user_info", "user_id", id.c_str(), "user_pw", pw.c_str());
						PGresult *respwcheck = PQexec(pCon, DML.c_str()); // DML SEND;

						if (PQntuples(respwcheck) > 0)
						{

							/* 1. ????????? ????????? ???????????? ???????????????  ???????????? ?????? */
							if (dbmynickname == nickname && dbmyphone == phone)
							{
								sendroot["action"] = "changemyinfo";
								sendroot["result"] = "true";
								sendroot["msg"] = "???????????? ????????? ?????????????????????";
							}

							/* 2. ?????? ???????????? ??????????????? ???????????? ??? ????????? ?????? */
							else if ((dbmynickname != nickname) && PQntuples(restotalnickname) > 0 || (dbmyphone != phone) && PQntuples(restotalphone) > 0)
							{
								sendroot["action"] = "changemyinfo";
								sendroot["result"] = "false";

								if ((dbmynickname != nickname) && PQntuples(restotalnickname) > 0)
									sendroot["msg"] = "????????? ??????????????????";
								else if ((dbmyphone != phone) && PQntuples(restotalphone) > 0)
									sendroot["msg"] = "????????? ?????????????????????";
							}

							/* 3. ???????????? ???????????? ?????? */
							else if (((dbmynickname != nickname) && (dbmyphone == phone)) && PQntuples(restotalnickname) == 0 && PQntuples(restotalphone) > 0)
							{
								DML = DML_Update(5, "user_info", "user_nickname", nickname.c_str(),
												 "user_id", id.c_str());
								PGresult *resupdate = PQexec(pCon, DML.c_str()); // DML SEND;

								sendroot["action"] = "changemyinfo";
								sendroot["result"] = "true";
								sendroot["msg"] = "???????????? ????????? ?????????????????????";
							}

							/* 4. ??????????????? ???????????? ?????? */
							else if (((dbmynickname == nickname) && (dbmyphone != phone)) && PQntuples(restotalnickname) > 0 && PQntuples(restotalphone) == 0)
							{
								DML = DML_Update(5, "user_info", "user_phone", phone.c_str(),
												 "user_id", id.c_str());
								PGresult *resupdate = PQexec(pCon, DML.c_str()); // DML SEND;

								sendroot["action"] = "changemyinfo";
								sendroot["result"] = "true";
								sendroot["msg"] = "???????????? ????????? ?????????????????????";
							}

							/* 5. ???????????? ???????????? ??? ??? ???????????? ?????? */
							else if (((dbmynickname != nickname) && (dbmyphone != phone)) && PQntuples(restotalnickname) == 0 && PQntuples(restotalphone) == 0)
							{
								DML = "update user_info set user_nickname = '" + nickname + "', user_phone = '" + phone + "' where user_id = '" + id + "';";
								PGresult *resupdate = PQexec(pCon, DML.c_str()); // DML SEND;

								sendroot["action"] = "changemyinfo";
								sendroot["result"] = "true";
								sendroot["msg"] = "???????????? ????????? ?????????????????????";
							}
						}

						/* 6. ?????? ????????? ???????????? ???????????? */
						else
						{
							sendroot["action"] = "changemyinfo";
							sendroot["result"] = "false";
							sendroot["msg"] = "??????????????? ???????????? ????????????";
						}

						data.msg.clear();
						data.msg = writer.write(sendroot);
						data.size = data.msg.size();

					} /* end changemyinfo */

					else if (action == "deleteaccount")
					{
						string nickname = recvroot["nickname"].asString();
						string id = c[index].id;

						/* 1. ?????? ????????? ???????????? ?????? */
						DML = DML_Select(4, "user_num", "user_info", "user_id", id.c_str());
						PGresult *resusernum = PQexec(pCon, DML.c_str()); // DML SEND;

						if (PQntuples(resusernum) == 0)
							cout << "select user_num error" << endl;

						else
						{
							int myusernum = stoi(PQgetvalue(resusernum, 0, 0));
							/* 2. ?????? ????????? ????????? ??????????????? ?????? */
							DML = "select friends_user_num from friends_info where user_num = " + to_string(myusernum) + ";";
							PGresult *resfriendusernum = PQexec(pCon, DML.c_str()); // DML SEND;

							/* ?????? ????????? ????????? ???????????? ?????? ?????? */
							if (PQntuples(resfriendusernum) == 0)
							{
								/* 4. ?????? ????????? ????????? ???????????? ?????? ????????? ??????????????? ?????? */
								DML = "select user_num from friends_info where friends_user_num = " + to_string(myusernum) + ";";
								PGresult *resselect = PQexec(pCon, DML.c_str()); // DML SEND;

								/* ?????? ????????? ????????? ???????????? ?????? ????????? ???????????? ?????? ?????? */
								if (PQntuples(resselect) == 0)
								{
									/* 4. ???????????? ?????? */
									DML = "delete from user_info where user_num = " + to_string(myusernum) + ";";
									PGresult *resuserdelete = PQexec(pCon, DML.c_str()); // DML SEND;

									if (PQresultStatus(resuserdelete) != PGRES_COMMAND_OK)
										cout << "user delete fail" << endl;
									else
									{
										sendroot["action"] = "deleteaccount";
										sendroot["result"] = "true";
										sendroot["msg"] = "??????????????? ?????????????????????";
									}
								}

								/* ?????? ????????? ????????? ???????????? ?????? ????????? ????????? ?????? */
								else
								{
									int friendusernum = stoi(PQgetvalue(resselect, 0, 0));
									DML = "delete from friends_info where user_num = " + to_string(friendusernum) + " and " + "friends_user_num = " + to_string(myusernum) + ";";
									PGresult *resfrienddelete = PQexec(pCon, DML.c_str()); // DML SEND;

									/* ?????? ????????? ????????? ???????????? ?????? ??????????????? ?????? ????????? ???????????? ????????? ?????? */
									if (PQresultStatus(resfrienddelete) != PGRES_COMMAND_OK)
										cout << "friend user delete fail" << endl;
									/* ?????? ????????? ????????? ???????????? ?????? ??????????????? ?????? ????????? ???????????? ?????? */
									else
									{
										/* 4. ???????????? ?????? */
										DML = "delete from user_info where user_num = " + to_string(myusernum) + ";";
										PGresult *resuserdelete = PQexec(pCon, DML.c_str()); // DML SEND;

										if (PQresultStatus(resuserdelete) != PGRES_COMMAND_OK)
											cout << "user delete fail" << endl;
										else
										{
											sendroot["action"] = "deleteaccount";
											sendroot["result"] = "true";
											sendroot["msg"] = "??????????????? ?????????????????????";
										}
									}
								}
							}

							/* ?????? ????????? ????????? ???????????? ?????? */
							else
							{
								/* 3. ?????? ????????? ???????????? ?????? */
								DML = "delete from friends_info where user_num = " + to_string(myusernum) + ";";
								PGresult *resmydeletefriend = PQexec(pCon, DML.c_str()); // DML SEND;

								/* ?????? ????????? ?????? ?????? ????????? ???????????? ?????? */
								if (PQresultStatus(resmydeletefriend) != PGRES_COMMAND_OK)
									cout << "delete friend error" << endl;
								/* ?????? ????????? ?????? ?????? ????????? ???????????? ?????? */
								else
								{
									/* 4. ?????? ????????? ????????? ???????????? ?????? ????????? ??????????????? ?????? */
									DML = "select user_num from friends_info where friends_user_num = " + to_string(myusernum) + ";";
									PGresult *resselect = PQexec(pCon, DML.c_str()); // DML SEND;

									/* ?????? ????????? ????????? ???????????? ?????? ????????? ???????????? ?????? ?????? */
									if (PQntuples(resselect) == 0)
									{
										/* 4. ???????????? ?????? */
										DML = "delete from user_info where user_num = " + to_string(myusernum) + ";";
										PGresult *resuserdelete = PQexec(pCon, DML.c_str()); // DML SEND;

										if (PQresultStatus(resuserdelete) != PGRES_COMMAND_OK)
											cout << "user delete fail" << endl;
										else
										{
											sendroot["action"] = "deleteaccount";
											sendroot["result"] = "true";
											sendroot["msg"] = "??????????????? ?????????????????????";
										}
									}

									/* ?????? ????????? ????????? ???????????? ?????? ????????? ????????? ?????? */
									else
									{
										int friendusernum = stoi(PQgetvalue(resselect, 0, 0));
										DML = "delete from friends_info where user_num = " + to_string(friendusernum) + " and " + "friends_user_num = " + to_string(myusernum) + ";";
										PGresult *resfrienddelete = PQexec(pCon, DML.c_str()); // DML SEND;

										/* ?????? ????????? ????????? ???????????? ?????? ??????????????? ?????? ????????? ???????????? ????????? ?????? */
										if (PQresultStatus(resfrienddelete) != PGRES_COMMAND_OK)
											cout << "friend user delete fail" << endl;
										/* ?????? ????????? ????????? ???????????? ?????? ??????????????? ?????? ????????? ???????????? ?????? */
										else
										{
											/* 4. ???????????? ?????? */
											DML = "delete from user_info where user_num = " + to_string(myusernum) + ";";
											PGresult *resuserdelete = PQexec(pCon, DML.c_str()); // DML SEND;

											if (PQresultStatus(resuserdelete) != PGRES_COMMAND_OK)
												cout << "user delete fail" << endl;
											else
											{
												sendroot["action"] = "deleteaccount";
												sendroot["result"] = "true";
												sendroot["msg"] = "??????????????? ?????????????????????";
											}
										}
									}
								}
							}
						}

						data.msg.clear();
						data.msg = writer.write(sendroot);
						data.size = data.msg.size();

					} /* end deleteaccount */
					else if (action == "logout")
					{
						string nickname = recvroot["nickname"].asString();
						// ????????? ?????? ?????????????????? ????????????
						DML = "update user_info set user_state = " + to_string(0) + " where user_nickname = '" + nickname + "';";
						PGresult *resUpdate = PQexec(pCon, DML.c_str()); // DML SEND;
						sendroot["action"] = "logout";
						sendroot["msg"] = "??????????????? ??????????????????";
						data.msg.clear();
						data.msg = writer.write(sendroot);
						data.size = data.msg.size();
						PQclear(resUpdate);
					}
				}
				if (parseSuccessful)
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

void rcv(void *p)
{
	Trcv *pp = (Trcv *)p;

	int read_len = SSL_read(pp->ssl, &pp->data.size, sizeof(int));
	if (read_len == 0)
	{
		// cout << "1" << endl;
		// cout << SSL_shutdown(pp->ssl) << endl;
		// cout << "2" << endl;
		// SSL_free(pp->ssl);
		// cout << "3" << endl;
		pp->fd_arr = -1;
		(void)close(pp->fd);
		printf("client closed(%d)\n", pp->fd);
		FD_CLR(pp->fd, &pp->fd_cpy_reads);
	}
	else if (read_len > 0)
	{
		cout << "Receive Body Size : " << pp->data.size << endl;

		pp->data.msg.resize(pp->data.size);
		int ret_body_size = SSL_read(pp->ssl, &pp->data.msg[0], pp->data.size);
		if (ret_body_size > 0)
		{
			cout << "Receive Message : " << pp->data.msg << endl;
			pp->Queue.push(pp->data.msg);
		}
	}
}