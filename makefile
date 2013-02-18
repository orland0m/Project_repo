#Compiler
CC=g++
#Compiler flags
CFLAGS=-c -Wall -Werror
LDFLAGS=
#Source files
SOURCES=http-proxy.cc receive-timeout.cc cache.cc connection-handler.cc http-headers.cc http-request.cc http-response.cc web-request.cc
#Objects
OBJECTS=$(SOURCES:.cc=.o)
#Boost libraries
BOOST_PATH= -L/u/cs/grad/yingdi/boost/lib
#Boost link flags
BOOST_LINK_FLAGS=-lboost_filesystem-mt -lboost_date_time-mt
#Build directory
BIN_DIR=build/
#File containing object files
OBJ_FIR=$(BIN_DIR)obj/
#Executable name
EXECUTABLE=$(BIN_DIR)http-proxy

all: $(SOURCES) $(EXECUTABLE) 
	
$(EXECUTABLE): $(OBJECTS) 
	mkdir $(BIN_DIR) && touch $@
	$(CC) $(BOOST_PATH) $(LDFLAGS) $(OBJECTS) -o $@ $(BOOST_LINK_FLAGS)
	mkdir $(OBJ_FIR) && mv *o $(OBJ_FIR)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf build 2>/dev/null