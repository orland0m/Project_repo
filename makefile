MKDIR_P = mkdir -p
.PHONY: build
CC=g++
CFLAGS=-c -Wall -Werror
LDFLAGS=
SOURCES=cache.cc connection-handler.cc http-headers.cc http-proxy.cc http-request.cc http-response.cc web-request.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=build/http-proxy

all: $(SOURCES) $(EXECUTABLE) build

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *o build/http-proxy