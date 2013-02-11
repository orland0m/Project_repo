#include <iostream>
#include <fstream>
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

void ProcessRequest(string rq){
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length()); // parse request
	string response = GetFromCache(request, 0); // get non expired file from cache
	if(response.length()>0){
		cout << "Request in cache/" << request->GetHost() << request->GetPath() << endl;
		return;
	}
	cout << "Making remote request..." << endl;
	
	string destPort = "80"; // port, BTW I'm not sure how to get the port number from the request
	string destHost = string(request->GetHost()); // host URL
	
	int socket = serverNegotiateClientConnection(destHost.c_str(), destPort.c_str());//created socket
	
	response = GetFromRemoteServer(request, socket); //requesting to remote server
	delete request;
	cout << "Response received! length: "<< endl << response.length() <<endl;
	// "response" should contain a file ready to be sent to the client, even if there was an error
}

string putCRLF(char const * str){
	string tmp = "";
	for(int i=0; str[i]!='\0'; i++){
		if(str[i]=='\\'&&str[i+1]=='r'){
			tmp+='\r';
			i++;
			continue;
		}
		if(str[i]=='\\'&&str[i+1]=='n'){
			tmp+='\n';
			i++;
			continue;
		}
		tmp+=str[i];
	}
	return tmp;
}

int ProcessFile (string name){
	string line;
	std::ifstream myfile (name.c_str());
	if (myfile.is_open()){
    	while ( myfile.good() ){
    		getline(myfile,line);
    		cout << line << endl;
    		ProcessRequest(putCRLF(line.c_str()));
    		cout << endl;
    	}
    	myfile.close();
	}else cout << "Unable to open file " << name;
	return 0;
}