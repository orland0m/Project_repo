#ifndef CONNECTION-HANDLER
#define CONNECTION-HANDLER

//define some constants
#define LISTENING_BACKLOG 100
#define LISTENING_PORT "14805" //port for http proxy server to listen on
#define REMOTE_PORT "80" //remote server port to retrieve data from

//C++ libraries
#include <string>
#include <iostream>

//C networking libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//start the server listening component on defined port
//ie: int socket = serverStartListening(LISTENING_PORT)
int serverStartListening(const char* port)

//creates client connection with the specified host and port
int serverNegotiateClientConnection(const char* host, const char* port)

//request data and retrieve from remote host
int serverRetrieveRemoteData(int remote, std::string& data)

#endif
