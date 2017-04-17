CXXFLAGS=-g -std=c++11
LDLIBS=-ltins -lmicrohttpd
LD=g++

all: netwhere netwhere_test

netwhere: netwhere.o main.o
	/usr/local/bin/g++ $(LDFLAGS) $(LDLIBS) -o $@ $^

netwhere_test: netwhere_test.o netwhere.o
	/usr/local/bin/g++ $(CXXFLAGS) $(LDFLAGS) $^ -lboost_unit_test_framework $(LDLIBS) -o $@

