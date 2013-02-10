#include "cache.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define TIME_OUT 4

using namespace std;

string GetFromRemoteServer(HttpRequest * request, int& sockfd){
	string response = GetErrorPage(500); // have a response with an error message ready
	size_t msg_len = request->GetTotalLength(); // message length
	char * msg = new char[msg_len]; // message buffer
	msg[0] = '\0'; // empty string
	request->FormatRequest(msg); // fill buffer
	string tmp = "";
	
	cout << "Sending: {"<< endl << msg << endl << "}" << endl; // debug output
	int error = 0; // set to 1 if there was a problem
	
	int bytes_sent = send(sockfd, msg, msg_len, 0);
	if(bytes_sent>0){
		msg = new char[BUFFER_SIZE];
		int bytes_read = recv(sockfd, msg, BUFFER_SIZE, 0);
		if(bytes_read>0){
			tmp += string(msg);
			while(bytes_read>0){
				msg = new char[BUFFER_SIZE];
				msg[0] = '\0';
				bytes_read = recv(sockfd, msg, BUFFER_SIZE, 0);
				tmp += string(msg);
			}
		}else{
			error = 1;
		}
	}else{
		cerr << "Error sending" << endl;
		error = 1;
	}
		
	if(!error&&tmp.length()>0) response = tmp;
	
	if(!error){
		response = SaveToCache(response, request->GetHost()+request->GetPath()); 
	}else{
		sockfd = -1;
	}
	return response;
}