.PHONY: all clean test

CC=
CFLAGS=

CXX=g++
CXXFLAGS=-Wall -Wextra -ansi -pedantic -I. -ggdb

export INCLUDES=-I$(shell pwd)

all : interface libmwl.a client server module thread_test

interface : tool/interface

tool/interface :
	$(MAKE) -C tool interface

libmwl.a : mwl/libmwl.a

mwl/libmwl.a :
	$(MAKE) -C mwl

clean :
	rm -f *.o test.cpp test.hpp client server module
	rm -f thread_test
	$(MAKE) -C tool clean
	$(MAKE) -C mwl clean

thread_test : thread_test.o mwl/libmwl.a
	$(CXX) -o $@ thread_test.o -Lmwl -lmwl -lpthread

test.cpp test.hpp : tool/interface test.interface
	tool/interface --output=test test.interface

client : test.o client.o mwl/libmwl.a
	$(CXX) -o $@ test.o client.o -Lmwl -lmwl

server : test.o server.o mwl/libmwl.a
	$(CXX) -o $@ test.o server.o -Lmwl -lmwl -lpthread

module : test.o module.o mwl/libmwl.a
	$(CXX) -o $@ test.o module.o -Lmwl -lmwl -lpthread

%.o : %.c
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCLUDES)

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCLUDES)

%.o : %.cc
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCLUDES)

