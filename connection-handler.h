#ifndef _CONNECTION_HANDLER_
#define _CONNECTION_HANDLER_

//define some constants
#define LISTENING_BACKLOG 100 //maximum listening port requests allowed
#define LISTENING_PORT "14805" //port for http proxy server to listen on
#define REMOTE_PORT "80" //remote server port to retrieve data from
#define BUFFER_SIZE 2048 //buffer size for incoming data connection

//C++ libraries
#include <string>
#include <iostream>

//C libraries
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//C networking libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//provided skeletons
#include "http-response.h"
#include "http-request.h"

//start the server listening component on defined port
//ie: int socket = serverStartListening(LISTENING_PORT)
int serverStartListen(const char* port);

//creates client connection with the specified host and port
int serverNegotiateClientConnection(const char* host, const char* port);

//request data and retrieve from remote host
int serverRetrieveRemoteData(int remote, std::string& data);

//receives HTTP data from client and requests data from remote server
int serverClientConnection(int client, pthread_mutex_t *mutex);

#endif
