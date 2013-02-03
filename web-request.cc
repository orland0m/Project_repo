//#import "cache.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_LISTEN 5 // is the number of queued responses a web client will have

using namespace std;

HttpResponse * GetFromRemoteServer(HttpRequest * request, int socketFd){ // GetErrorPage
	HttpResponse * response = NULL;
	
	return response;
}

void SetUpConnection(const char * server, const * char port, int * socket){
	(*socket) = -1;
	int status, s;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	status = getaddrinfo(server, port, &hints, &servinfo);
	if(status){
		fprintf(stderr, "getaddrinfo error: %s\nHost: %s\nPort: %s\n", gai_strerror(status),server,port);
		return;
	}
	status = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(status<0){
		fprintf(stderr, "web-request.cc:SetUpConnection error: %s\nHost: %s\nPort: %s\n", "Could not create socket",server,port);
		return;
	}
	s = status;
	/*status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if(status<0){
		fprintf(stderr, "web-request.cc:SetUpConnection error: %s\nHost: %s\nPort: %s\n", "Could not bind socket",server,port);
		return;
	}*/ // omitted to let connect use the port it wants, we are the client anyway
	status = connect(s, servinfo->ai_addr, servinfo->ai_addrlen);
	if(status<0){
		fprintf(stderr, "web-request.cc:SetUpConnection error: %s\nHost: %s\nPort: %s\n", "Could not connect socket",server,port);
		return;
	}
	status = listen(s, MAX_LISTEN);
	if(status<0){
		fprintf(stderr, "web-request.cc:SetUpConnection error: %s\nHost: %s\nPort: %s\n", "Socket can not listen",server,port);
		return;
	}
	(*socket) = &s;
	//freeaddrinfo(servinfo); // see if it's necessary at this point
}