.PHONY: all clean test

OBJECTS=\
	LocalSocketStream.o \
	LocalSocketStreamServer.o \
	Pipe.o \
	Mutex.o \
	ConditionVar.o \
	Executor.o \
	Thread.o \
	ThreadBase.o \
	Selector.o \
	Server.o \
	Channel.o \
	ModuleBase.o \
	ModuleServer.o \
	Message.o

CC=
CFLAGS=

CXX=g++
CXXFLAGS=-Wall -Wextra -ansi -pedantic -ggdb

all : libmwl.a

libmwl.a : $(OBJECTS)
	ar rcs libmwl.a $(OBJECTS)

clean :
	rm -f *.o *.a

%.o : %.c
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCLUDES)

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCLUDES)

%.o : %.cc
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCLUDES)

