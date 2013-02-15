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

using namespace std;
pthread_mutex_t * mutex;

string ProcessRequest(string rq){
	HttpRequest * request = new HttpRequest;
	request -> ParseRequest(rq.c_str(), rq.length()); // parse request
	cout << "Searching in cache" << endl;
	string response = GetFromCache(request, 0, mutex); // get non expired file from cache
	if(response.length()>0){
		return response;
	}
	cout << "Making remote request..." << endl;
	char pTMP [20];
	memset(pTMP, '\0',20);
	sprintf(pTMP,"%d",request -> GetPort());
	string destPort = string(pTMP);
	string destHost = string(request->GetHost()); // host URL
	
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

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(){
	int socket = serverStartListen(LISTENING_PORT);
	
	if(socket<0){
		return 1;
	}
	cout << "Listening..." << endl;
	for(;;){
		struct sockaddr_storage their_addr;
		char s[INET6_ADDRSTRLEN];
		socklen_t sin_size = sizeof their_addr;
		
		int client_fd = accept(socket, (struct sockaddr *)&their_addr, &sin_size);
		if(client_fd<0){
			return 1;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		
		char * tmp = new char[1024];
		int counter = 0;
		memset(tmp,'\0',1024);
		int * endFlags = new int[3];
		int bytes_read = 0;
		char * msg;
		int error = 0;
		while(1){ // read header only, eventually it has to break
			msg = new char[1];
			bytes_read = recv(client_fd, msg, 1, 0);// read header by byte
			if(bytes_read==1){
				tmp[counter++] = msg[0];
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
		cout << tmp << endl;
		string response = ProcessRequest(string(tmp));
		cout << response << endl;
		
	}
	return 0;
}