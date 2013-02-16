#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>

//project wireframe
#include "http-request.h"
#include "http-headers.h"
#include "http-response.h"

//custom defined files
#include "web-request.h" /* GetFromRemoteServer */
#include "cache.h" /* GetFromCache */
#include "connection-handler.h" //handles http connections

// HEADER FILES

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>



// UNIVERSAL DEFINITIONS
//#define _WIN32_WINNT 0x501
#define MYPORT "4234"     // the port users will be connecting to
#define BACKLOG 2         // how many pending connections queue will hold
#define MAXDATASIZE 1024  // the maximum size of data being read
#define MAXBUFFER 72000   // the maximum size of the buffer
#define CONCURRENT 1

using namespace std; 

using namespace std;
pthread_mutex_t * mutex;

string ProcessRequest(string rq, int& closeCon){
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length()); // parse request
	if((string("1.0")).compare(request->GetVersion())==0){
		closeCon = 1;
	}
	string response = GetFromCache(request, 0, mutex); // get non expired file from cache
	if(response.length()>0){
		cout<< getpid() << ": Response in cache..." << endl;
		return response;
	}
	
	cout<< getpid() << ": Making remote request..." << endl;
	char cPort [20];
	memset(cPort, '\0',20);
	sprintf(cPort,"%d",request -> GetPort());
	
	int socket = serverNegotiateClientConnection(request->GetHost().c_str(), cPort);//created socket
	response = GetFromRemoteServer(request, socket, mutex); //requesting to remote server
	if(socket<0){
		closeCon = 1;
	}
	close(socket);
	delete request;
	return response;
}


void fun(int client_fd){
	receive:
	int close_connection = 0;
	string tmp = "";
	int * endFlags = new int[3];
	int bytes_read = 0;
	char * msg;
	int error = 0;
	while(1){ // read header only, eventually it has to break
		msg = new char[1];
		//select();
		bytes_read = recv(client_fd, msg, 1, 0);// read header by byte
		if(bytes_read==1){
			tmp += msg[0];
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
	if(tmp.length()<2){
		cout<< getpid() << ": Error reading from socket" << endl;
		error = 1;
	}
	if(!error){
		string response = ProcessRequest(tmp, close_connection);
		cout<< getpid() << ": Sending response to client..." << endl;
		int bytes_sent = send(client_fd, response.c_str(), response.length(), 0);
		if(bytes_sent<0){ 
			cout<< getpid() << ": Error sending response to client" << endl;
			close(client_fd);
		}else if(close_connection){
			cout << getpid() << ": Client wants to close the connection" << endl;
			close(client_fd);
		}else{
			goto receive;
		}
	}else{
		cout << getpid() << ": Error receiving data" << endl;
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



// Same function as recv; with a timeout limit (param timeout)

// Retval: -1: error, -2: timeout

int recvtimeout(int s, char *buf, int len, int timeout){
    fd_set fds;
    int n;
    struct timeval tv;
    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    // set up the struct timeval for the timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    // wait until timeout or data received
    n = select(s+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error
    // data must be here, so do a normal recv()
    return recv(s, buf, len, 0);
}



// Main function

int main (int argc, char *argv[]){
	// Counter + limit to keep number of clients to 10
	mutex = new pthread_mutex_t;
	int tot_connect = 0;
	int max_connect = CONCURRENT;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, nbytes;
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
		addr_size = sizeof their_addr;
		char s[INET6_ADDRSTRLEN];
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if (new_fd == -1){
			continue;
		}
		else{
			if (tot_connect < max_connect)
				tot_connect++;
			else{
				//send_reject_message(new_fd);
				close(new_fd); 
			}
		}
		pid_t pid = fork();
		//int status;

		while ((pid=waitpid(-1, &status, WNOHANG)) != -1){
			tot_connect--;
		}

		//	tot_connect--;
		if (pid == 0){
			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
			cout<< getpid() << ": Incoming connection accepted: " << *s << endl;
			nbytes = 0;
			//char buf[MAXDATASIZE];
			//string bigBuf;
			fun(new_fd);
			/*while ((nbytes = recvtimeout(new_fd, buf, MAXDATASIZE-1, 60)) > 0)

			{

				cout << "Message is: " << buf << endl;

				bigBuf.append(buf,nbytes);

				if((bigBuf.find("\r\n\r\n"))!=string::npos)

					break;

			}*/

			if( nbytes == -1){
				perror("recv");
				break;
			}
			close(new_fd);
			//if (send(new_fd, "This is the output to the client.\n", 35, 0) == -1)
			//	perror("send");
			exit(0);
		}
		if (pid > 0)
			close(new_fd);
	}
	return 0;
}
