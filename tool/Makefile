.PHONY: all clean

CC=
CFLAGS=

CXX=g++
CXXFLAGS=-Wall -Wextra -ansi -pedantic -I. -ggdb

all : interface

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
	rm -f interface interface.tab.* lex.yy.* *.o

%.o : %.c
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o : %.cc
	$(CXX) -o $@ -c $< $(CXXFLAGS)

