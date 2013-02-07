/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "web-request.h"
#include "cache.h"
#include "http-request.h"
#include "http-response.h"

using namespace std;

int main (int argc, char *argv[]){
	string rq = "GET http://www.google.com/ HTTP/1.1\r\n\r\n";
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length());
	HttpResponse * r;
	r = GetFromCache(request, 0); // get non expired file from cache
	if(r){
		cout << "Bingo! your file is in: cache/" << request->GetHost() << request->GetPath() << endl;
		return 0;
	}else{
		cout << "Oops... This is embarrassing. We don't have that file yet" << endl;
	}
	cout << "Making remote request..." << endl;
	char * data;
	r = GetFromRemoteServer(request,0); //requesting to remote server, the second argument should be a socket file descriptor
	data = new char[r->GetTotalLength()];
	r -> FormatResponse(data);
	cout << "Response arrived: " << data <<endl;
	return 0;
}
