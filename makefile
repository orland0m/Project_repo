CC=g++
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=http-proxy.cc receive-timeout.cc cache.cc connection-handler.cc http-headers.cc http-request.cc http-response.cc web-request.cc
#SOURCES=tester.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=build/http-proxy
BOOST_PATH= -L/u/cs/grad/yingdi/boost/lib
BOOST_LINK_FLAGS=-lboost_filesystem-mt -lboost_date_time-mt 

build/http-proxy: build
    cd build && touch http-proxy

dir:
    mkdir build

all: $(SOURCES) $(EXECUTABLE) 

	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(BOOST_PATH) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LINK_FLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *o build/http-proxy