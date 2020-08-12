SOURCE  := $(wildcard src/*.cpp)
OBJS    := $(patsubst %.cpp,%.o,$(SOURCE))

TARGET  := server
CC      := g++
LIBS    := -lpthread
CFLAGS  := -std=c++11 -g -Wall -O0

all : $(TARGET) client

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
client : Client/HTTPClient.cpp
	$(CC) $(CFLAGS) -o $@ $<

.PHONY:clean
clean :
	rm -rf $(OBJS) client server