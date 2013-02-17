// C++ libraries

#include <iostream>
#include <fstream>
#include <string>

// Linux libraries
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// C libraries
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

//project wireframe
#include "http-request.h"
#include "http-headers.h"
#include "http-response.h"

//custom defined files
#include "web-request.h" /* GetFromRemoteServer */
#include "cache.h" /* GetFromCache */
#include "connection-handler.h" //handles http connections
#include "receive-timeout.h" /* recvtimeout */

#define MYPORT "14805"     // the port users will be connecting to
#define BACKLOG 2         // how many pending connections queue will hold
#define MAXDATASIZE 1024  // the maximum size of data being read
#define MAXBUFFER 72000   // the maximum size of the buffer
#define CONCURRENT 10

using namespace std; 

/**
	ProcessRequest's contract
	IN: string: http-request, int = 0
	OUT: string: http-response
	SIDE EFFECT: If there was some kind of error in the socket, or the http version is 1.0, 
	this function will set the second argument to 1. Meaning the socket with the client
	has to be closed.
*/
string ProcessRequest(string rq, int& closeCon){
	try{
		// parse request
		HttpRequest * request = new HttpRequest;
		request -> ParseRequest(rq.c_str(), rq.length()); // parse request
		if((string("1.0")).compare(request->GetVersion())==0){ // version = 1.0
			closeCon = 1;
		}
		string response = GetFromCache(request, 0); // get non expired file from cache
		if(response.length()>0){
			cout<< getpid() << ": Response in cache..." << endl;
			delete request;
			return response;
		}
		// Content wasn't in cache
		cout<< getpid() << ": Making remote request..." << endl;
		// get port
		char cPort [20];
		memset(cPort, '\0',20);
		sprintf(cPort,"%d",request -> GetPort());
		// Create socket with remote server
		int socket = serverNegotiateClientConnection(request->GetHost().c_str(), cPort);
		//requesting to remote server
		response = GetFromRemoteServer(request, socket);
		cout<< getpid() << ": Response received" << endl;
		if(socket<0){ // Error using created socket
			closeCon = 1;
		}
		close(socket);
		delete request;
		cout<< getpid() << ": Serving response" << endl;
		return response;
	}catch(...){
	}
	return GetErrorPage("There was an error processing your request");
}


/**
	This function will handle any open client connection from beginning to end
*/

void HandleClientConnection(int client_fd){
	// In case the protocol is 1.1
	receive: 
	int close_connection = 0; // flag to close connection
	string tmp = ""; // temporary to store response
	int endFlags[3]; // flags used to read header only
	endFlags[0] = endFlags[1] = endFlags[2] = 0; // init flags
	int bytes_read = 0;
	char msg[] = {'\0'}; // aux to read header by byte
	int error = 0;
	while(1){ // read header only, eventually it has to break
		msg[0] = '\0';
		bytes_read = recvtimeout(client_fd, msg, 1);// read header by byte
		if(bytes_read==1){
			tmp += msg[0];
			// check if header has ended yet
			if(endFlags[0]&&endFlags[1]&&endFlags[2]&&msg[0]=='\n'){
				break;
			}else if(endFlags[0]&&endFlags[1]&&msg[0]=='\r'){
				endFlags[2] = 1;
			}else if(endFlags[0]&&msg[0]=='\n'){
				endFlags[1] = 1;
			}else if(msg[0]=='\r'){
				endFlags[0] = 1;
			}else{
				endFlags[0] = endFlags[1] = endFlags[2] = 0;
			}
		}else{
			error = 1;
			break;
		}
	}
	// if there was an error reading from the socket
	if(tmp.length()<1){
		error = 1;
	}
	if(!error){
		// Process this request
		string response = ProcessRequest(tmp, close_connection);
		cout<< getpid() << ": Sending response to client..." << endl;
		int bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
		if(bytes_sent<0){ 
			cout<< getpid() << ": Error sending response to client" << endl;
			close(client_fd);
		}else if(close_connection){
			cout << getpid() << ": Client wants to close the connection" << endl;
			close(client_fd);
		}else{ // http 1.1
			goto receive;
		}
	}else{
		cout << getpid() << ": Socket failed. Connection closed" << endl;
		close(client_fd);
	}
}

//
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



// Main function

int main (int argc, char *argv[]){
	// Counter + limit to keep number of clients to 10
	
	pid_t p_list [CONCURRENT];
	for(int i=0; i<CONCURRENT; i++){
		p_list[i] = -1;
	}
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd = 0, nbytes = 0;
	struct sockaddr_storage their_addr;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, MYPORT, &hints, &res) != 0){
	cout << "-ERROR: Could not get address info" << endl;
		return 1;
	}
    // Create socket, bind the socket, listen and accept up to 10 incoming connections
	struct addrinfo *loop;
	for (loop = res; loop != NULL; loop = loop->ai_next){
		// Create
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0){
			cout << "-ERROR: Could not create socket " << MYPORT << endl;
			return 1;
		}
		int ptrue = 1;
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ptrue, sizeof(int));
		// Bind

		int bindRet = bind(sockfd, res->ai_addr, res->ai_addrlen) != 0;
		cout << bindRet << endl;
		if (bindRet != 0){
			cout << "-ERROR: Could not bind socket " << MYPORT << endl;
			continue;
		}
		break;
	}
	if (loop == NULL){
		cout << "-ERROR: Failed to bind" << endl;
		return -2;
	}
	// Listen

    if (listen(sockfd, BACKLOG) != 0)
		cout << "-ERROR: Could not listen on port " << MYPORT << endl;
	cout << "+STATUS: Listening on port " << MYPORT << endl;

	freeaddrinfo(res); // No longer needed
	// Main loop (listening + accept)
	while(1){
		start_again:
		int free_pos = -1;
		addr_size = sizeof their_addr;
		char s[INET6_ADDRSTRLEN];
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if (new_fd == -1){
			continue;
		}else{
			for(int i=0; i<CONCURRENT; i++){
				if(p_list[i]<0){
					free_pos = i;
					break;
				}else{
					int status = 0;
					pid_t result = waitpid(p_list[i], &status, WNOHANG);
					if(result < 1){
						continue;
					}else{
						free_pos = i;
						break;
					}
				}
			}
			if(free_pos<0){
				close(new_fd);
				goto start_again;
			}
		}
		pid_t pid = fork();
		if(pid!=0) p_list[free_pos] = pid;
		
		if (pid == 0){
			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
			cout<< getpid() << ": Incoming connection accepted: " << *s << endl;
			nbytes = 0;
			
			HandleClientConnection(new_fd);

			if( nbytes == -1){
				perror("recv");
				break;
			}
			close(new_fd);
			exit(0);
		}
		if (pid > 0)
			close(new_fd);
	}
	return 0;
}
