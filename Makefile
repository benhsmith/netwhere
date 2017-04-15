CXXFLAGS=-g -std=c++11 -I ../boost_1_63_0 -I ../libtins-master/include
LDFLAGS=-L ../libtins-master/build/lib
LDLIBS=-ltins -lmicrohttpd
LD=g++

all: netwhere netwhere_test

netwhere: webservice.o netwhere.o
	/usr/local/bin/g++ $(LDFLAGS) $(LDLIBS) -o $@ $^

netwhere.o: netwhere.cpp

netwhere_test: netwhere_test.cpp
	/usr/local/bin/g++ $(CXXFLAGS) $< -lboost_unit_test_framework $(LDLIBS) -o $@

