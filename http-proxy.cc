/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//C++ libs
#include <iostream>
#include <string>

//project wireframe
#include "http-request.h"
#include "http-headers.h"
#include "http-response.h"

//custom defined files
#include "web-request.h" /* GetFromRemoteServer */
#include "cache.h" /* GetFromCache */
#include "connection-handler.h" //handles http connections

using namespace std;

int main (int argc, char *argv[]){
	/* cout << LISTENING_PORT << endl;
	//start server listening on specified port
	int serverConnection = serverStartListening(LISTENING_PORT);
	if (serverConnection < 0) {
		cout << "Cannot start listening on port " << LISTENING_PORT << endl;
		return 1;
	}*/
	
	string rq = "GET http://www.onenaught.com/posts/17/web-site-performance-expires-header HTTP/1.0\r\n\r\n"; // request
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length()); // parse request
	string response = GetFromCache(request, 0); // get non expired file from cache
	if(response.length()>0){
		cout << "Bingo! your file is in: cache/" << request->GetHost() << request->GetPath() << endl;
		return 0;
	}else{
		cout << "Oops... This is embarrassing. We don't have that file yet. It's not the end of the world though:" << endl;
	}
	cout << "Making remote request..." << endl;
	
	string destPort = "80"; // port
	string destHost = string(request->GetHost()); // host URL
	cout << destPort << ": Port" << endl;
	cout << destHost << ": Host" << endl;
	int socket = serverNegotiateClientConnection(destHost.c_str(), destPort.c_str());//created socket
	
	response = GetFromRemoteServer(request, socket); //requesting to remote server
	cout << "Response received: "<< endl << response.length() <<endl;
	// response should contain a file ready to be sent to the client, even if there was an error
	return 0;
}
