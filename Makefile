# In order to execute this "Makefile" just type "make" or "make ALL"
.PHONY: clean
OBJS	= dataServer.o remoteClient.o
OUT	= dataServer remoteClient
#DIRS	= named_pipes/* out/*
CPP	= g++
FLAGS	= -g -Wall -c

ALL: clean $(OBJS) $(OUT)

dataServer.o: dataServer.cpp
	$(CPP) $(FLAGS) dataServer.cpp
	
remoteClient.o: remoteClient.cpp
	$(CPP) $(FLAGS) remoteClient.cpp

dataServer:
	$(CPP) -g dataServer.o -o dataServer -fsanitize=address -g3

remoteClient:
	$(CPP) -g remoteClient.o -o remoteClient -fsanitize=address -g3

# Clean things
clean:
	rm -f $(OBJS) $(OUT) $(DIRS)
