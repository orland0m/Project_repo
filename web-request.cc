#include "cache.h"
#include <iostream>
#include <stdlib.h>
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
	int error = 0; // set to 1 if there was a problem
	
	int bytes_sent = send(sockfd, msg, msg_len, 0);
	if(bytes_sent>0){
		int bytes_read;
		int * endFlags = new int[3];
		endFlags[0] = endFlags[1] = endFlags[2] = 0;
		while(1){ // read header only, eventually it has to break
			msg = new char[1];
			bytes_read = recv(sockfd, msg, 1, 0);// read header by byte
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
		if(!error){
			cout << tmp << endl;
			HttpResponse * header = new HttpResponse;
			header -> ParseResponse(tmp.c_str(),tmp.length());
			string cntLengthStr = header -> FindHeader("Content-Length");
			int cntLength = atoi(cntLengthStr.c_str());
			if(cntLength>0){
				while(cntLength>0){
					msg = new char[BUFFER_SIZE];
					bytes_read = recv(sockfd, msg, BUFFER_SIZE, 0);
					if(bytes_read<1) break;
					tmp += string(msg, bytes_read);
					cntLength -= bytes_read;
				}
			}else{
				error = 1;
			}
		}
	}else{
		cerr << "Error sending request to server" << endl;
		error = 1;
	}
	
	if(!error&&tmp.length()>0){
		response = SaveToCache(tmp, request->GetHost()+request->GetPath()); 
	}else{
		sockfd = -1;
	}
	return response;
}