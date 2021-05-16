::Makefile for Assignment 3

# Update following lines as needed.
# You may change this file as you wish, as long as your Makefile can generate the target binary (webserver).

SERVER_SRC = src/webserver.c
SERVER_HDR = 

##############################

CC=gcc
CFLAGS=-I. -g
LDFLAGS= -lpthread -levent -levent_core
LIBS_PATH= -L/usr/local/lib

build: bin/webserver

bin/webserver: $(SERVER_SRC) $(SERVER_HDR) bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) $(LIBS_PATH) $(LDFLAGS)

.PHONY: clean build

bin:
	mkdir -p bin

clean:
	rm -rf bin
