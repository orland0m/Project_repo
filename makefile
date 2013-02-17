CC=g++
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=cache.cc connection-handler.cc http-headers.cc http-proxy.cc http-request.cc http-response.cc web-request.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=http-proxy

all: $(SOURCES) $(EXECUTABLE) 

	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *o build/http-proxy