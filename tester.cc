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
pthread_mutex_t * mutex;

string ProcessRequest(string rq){
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length()); // parse request
	string response = GetFromCache(request, 0, mutex); // get non expired file from cache
	if(response.length()>0){
		return response;
	}
	cout << "Making remote request..." << endl;
	char pTMP [20];
	memset(pTMP, '\0',20);
	sprintf(pTMP,"%d",request -> GetPort());
	string destPort = string(pTMP);//+request -> GetPort();
	cout << "Port: " << destPort << endl;
	string destHost = string(request->GetHost()); // host URL
	cout << "Host: " << destHost << endl;
	
	int socket = serverNegotiateClientConnection(destHost.c_str(), destPort.c_str());//created socket
	
	response = GetFromRemoteServer(request, socket, mutex); //requesting to remote server
	delete request;
	return response;
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

int main(){
	mutex = new pthread_mutex_t;
	if(mutex)
	ProcessFile("request.txt");
	else return 1;
	return 0;
}