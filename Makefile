# In order to execute this "Makefile" just type "make" or "make ALL"
.PHONY: clean
OBJS	= dataServer.o remoteClient.o
OUT	= dataServer remoteClient
#DIRS	= named_pipes/* out/*
CPP	= g++
FLAGS	= -g -Wall -c -lpthread

ALL: clean $(OBJS) $(OUT)

dataServer.o: dataServer.cpp
	$(CPP) $(FLAGS) dataServer.cpp
	
remoteClient.o: remoteClient.cpp
	$(CPP) $(FLAGS) remoteClient.cpp

dataServer:
	$(CPP) -g dataServer.o -o dataServer -fsanitize=address -g3

remoteClient:
	$(CPP) -g remoteClient.o -o remoteClient -fsanitize=address -g3

run_s:
	./dataServer -p 12500 -s 2 -q 2 -b 512

run_c1:
	./remoteClient -i 127.0.0.1 -p 12500 -d Server

run_c2:
	./remoteClient -i 127.0.0.1 -p 12500 -d Server2

# Clean things
clean:
	rm -f $(OBJS) $(OUT) $(DIRS)
