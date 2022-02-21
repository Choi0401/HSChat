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
	$(CC) -std=c++11 -o $(TARGET) $(OBJS) $(JSON) $(LIB) $(INC) $(SSL) $(CRY)

$(OBJS) : $(SRCS)
	$(CC) -std=c++11 -c -o $(OBJS) $(SRCS) $(LIB) $(INC)

