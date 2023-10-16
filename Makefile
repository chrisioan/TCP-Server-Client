# In order to execute this "Makefile" just type "make" or "make all"
.PHONY: all clean

CPP	= g++
CFLAGS	= -g -Wall -c -lpthread
LFLAGS	= -fsanitize=address -g3

SRC_DIR	= src
BUILD_DIR = build/release

OBJS	= $(BUILD_DIR)/dataServer.o $(BUILD_DIR)/remoteClient.o
EXECUTABLES	= $(BUILD_DIR)/dataServer $(BUILD_DIR)/remoteClient

all: dir $(OBJS) $(EXECUTABLES)

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/dataServer.o:
	$(CPP) $(CFLAGS) $(SRC_DIR)/dataServer.cpp -o $@

$(BUILD_DIR)/remoteClient.o:
	$(CPP) $(CFLAGS) $(SRC_DIR)/remoteClient.cpp -o $@

$(BUILD_DIR)/dataServer:
	$(CPP) $(LFLAGS) $(BUILD_DIR)/dataServer.o -o $@

$(BUILD_DIR)/remoteClient:
	$(CPP) $(LFLAGS) $(BUILD_DIR)/remoteClient.o -o $@

# Clean things
clean:
	rm -r -f $(BUILD_DIR)/*