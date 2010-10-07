.PHONY: all clean test

CC=
CFLAGS=

CXX=g++
CXXFLAGS=-Wall -Wextra -ansi -pedantic -I. -ggdb

all : interface client server module thread_test

thread_test : thread_test.o ThreadBase.o Thread.o Executor.o
	$(CXX) -o $@ $^ -lpthread

interface : interface.o model.o interface.tab.o lex.yy.o
	$(CXX) -o $@ $^

interface.o : interface.cpp interface.tab.c interface.tab.h

interface.tab.o : interface.tab.c

interface.tab.c interface.tab.h : interface.y
	bison -d -o $@ $<

lex.yy.c lex.yy.h : interface.l
	flex --header-file=lex.yy.h $^

lex.yy.o : lex.yy.c interface.tab.h
	$(CXX) -o $@ -c lex.yy.c $(CXXFLAGS)

clean :
	rm -f interface interface.tab.* lex.yy.* *.o test.cpp test.hpp client server module
	rm -f thread_test

test.cpp test.hpp : interface test.interface
	cat test.interface | ./interface

client : test.o client.o LocalSocketStream.o Message.o
	$(CXX) -o $@ $^

server : LocalSocketStream.o LocalSocketStreamServer.o Mutex.o Executor.o ThreadBase.o Selector.o test.o server.o Message.o
	$(CXX) -o $@ $^ -lpthread

module : LocalSocketStream.o LocalSocketStreamServer.o Mutex.o ConditionVar.o Executor.o Thread.o ThreadBase.o Selector.o test.o module.o Message.o
	$(CXX) -o $@ $^ -lpthread

%.o : %.c
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o : %.cc
	$(CXX) -o $@ -c $< $(CXXFLAGS)

