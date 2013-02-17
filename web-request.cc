// C libraries
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

// Linux libraries
#include <sys/types.h>
#include <sys/socket.h>

//C++ libraries
#include <iostream>

// User defined libraries
#include "cache.h" /* SaveToCache(data,url) */
#include "receive-timeout.h"

// Constants
#define BUFFER_SIZE 1024

using namespace std;

string GetFromRemoteServer(HttpRequest * request, int& sockfd){
	string response = GetErrorPage("Fatal error"); // have a response with an error message ready
	size_t msg_len = request->GetTotalLength(); // message length
	char * msg = new char[msg_len+1]; // message buffer
	memset(msg, '\0', msg_len+1);
	request->FormatRequest(msg); // fill buffer
	string tmp = "";
	int error = 0; // set to 1 if there was a problem
	
	int bytes_sent = send(sockfd, msg, msg_len, 0);
	delete msg;
	if(bytes_sent>0){
		int bytes_read;
		int endFlags[3];
		endFlags[0] = endFlags[1] = endFlags[2] = 0;
		msg = new char[1];
		while(1){ // read header only, eventually it has to break
			bytes_read = recvtimeout(sockfd, msg, 1);// read header by byte
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
		delete msg;
		if(!error){
			HttpResponse * header = new HttpResponse;
			header -> ParseResponse(tmp.c_str(),tmp.length());
			string cntLengthStr = header -> FindHeader("Content-Length");
			int cntLength = atoi(cntLengthStr.c_str());
			if(cntLength>0){
				while(cntLength>0){
					msg = new char[BUFFER_SIZE];
					bytes_read = recvtimeout(sockfd, msg, BUFFER_SIZE);
					if(bytes_read<1) break;
					tmp += string(msg, bytes_read);
					delete msg;
					cntLength -= bytes_read;
				}
			}
			delete header;
		}
	}else{
		cerr << getpid() << ": Error sending request to server" << endl;
		error = 1;
	}
	
	if(!error&&tmp.length()>0){
		response = SaveToCache(tmp, request->GetHost()+request->GetPath()); 
	}else{
		response = GetErrorPage("Error using socket");
		sockfd = -1;
	}
	return response;
}