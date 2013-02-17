CC=g++
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=tester.cc cache.cc connection-handler.cc http-headers.cc http-request.cc http-response.cc web-request.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=http-proxy
BOOST= -L/u/cs/grad/yingdi/boost/lib
BOOST_OP=-lboost_system

all: $(SOURCES) $(EXECUTABLE) 

	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(BOOST) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_OP)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *o build/http-proxy