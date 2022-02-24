CC              = g++
SRCS            = eserver.cpp
OBJS            = server.o
TARGET          = server
LIB        		= -lpq
JSON			= jsoncpp.cpp
INC             = -I/usr/include/postgresql
SSL				= -lssl
CRY				= -lcrypto

$(TARGET) : $(OBJS)
	$(CC) -std=c++14 -o $(TARGET) $(OBJS) $(JSON) $(LIB) $(INC) $(SSL) $(CRY) -pthread 

$(OBJS) : $(SRCS)
	$(CC) -std=c++14 -c -o $(OBJS) $(SRCS) $(LIB) $(INC)

