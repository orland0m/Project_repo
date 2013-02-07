/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "web-request.h" /* GetFromRemoteServer */
#include "cache.h" /* GetFromCache */
#include "http-request.h" /* */
#include "http-response.h"

using namespace std;

int main (int argc, char *argv[]){
	string rq = "GET http://www.google.com/ HTTP/1.1\r\n\r\n";
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length());
	string response;
	response = GetFromCache(request, 0); // get non expired file from cache
	if(response!=NULL){
		cout << "Bingo! your file is in: cache/" << request->GetHost() << request->GetPath() << endl;
		return 0;
	}else{
		cout << "Oops... This is embarrassing. We don't have that file yet" << endl;
	}
	cout << "Making remote request..." << endl;
	response = GetFromRemoteServer(request,0); //requesting to remote server, the second argument should be a socket file descriptor
	cout << "Response arrived: " << response <<endl;
	// response should contain a file ready to be sent to the client, even if there was an error
	return 0;
}
