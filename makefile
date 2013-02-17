CC=g++
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=http-proxy.cc receive-timeout.cc cache.cc connection-handler.cc http-headers.cc http-request.cc http-response.cc web-request.cc
OBJECTS=$(SOURCES:.cc=.o)
BOOST_PATH= -L/u/cs/grad/yingdi/boost/lib
BOOST_LINK_FLAGS=-lboost_filesystem-mt -lboost_date_time-mt
BIN_DIR=build/
EXECUTABLE=$(BIN_DIR)http-proxy


all: $(SOURCES) $(EXECUTABLE) 
	
$(EXECUTABLE): $(OBJECTS) 
	mkdir $(BIN_DIR) && touch $@
	$(CC) $(BOOST_PATH) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LINK_FLAGS)
	mkdir $(BIN_DIR)obj/ && mv *o $(BIN_DIR)obj/

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf build *o