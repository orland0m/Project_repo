#include "connection-handler.h"

using namespace std;

int serverStartListen(const char* port) {
	//create C library socket structs
	struct addrinfo hints, *res;

	//initialize hints struct
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	//getaddrinfo converts human readable text strings for hostnames
	//or IP addresses into dynamically allocated list of
	//struct addrinfo structures
	//getaddrinfo(char* hostname, const char* service
	//const struct addrinf *hints, struct addrinfo** res);
	//hints is addrinfo struct with service we are interested in
	//returns 0 if successful, negative if failed
	//res is the response pointer for addrinfo created
	int addrStatus = getaddrinfo(NULL, port, &hints, &res);
	
	if (addrStatus != 0) {
		cerr << "Server cannot get address info" << endl;
	}
	
	//loop through addrinfos in the response pointer
	struct addrinfo* current;
	int openSocket;
	for (current = res; current != NULL; current = current -> ai_next) {
		//create socket for addresses
		openSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (openSocket < 0) {
			cerr << "Server cannot open socket" << endl;
			continue;
		}
		
		//set socket options
	 	int one = 1;
		int optionStatus = setsockopt(openSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
		if (optionStatus < 0) {
			cerr << "Server cannot set socket options" << endl;
			continue;
		}
		
		//bind socket to port
		int bindStatus = bind(openSocket, res->ai_addr, res->ai_addrlen);
		if (bindStatus != 0) {
			//close socket
			close(optionStatus);
			cerr << "Server cannot bind socket" << endl;
			continue;
		}
		
		//only need to bind one. loop is only for error cases
		break;
	}
	
	//what if no binds happened
	if (current == NULL) {
		cerr << "Server failed all port binding attempts" << endl;
	}

	//garbage collect all used pointers
	free(res);

	//start listening on port
	if (listen(openSocket, LISTENING_BACKLOG) == -1) {
		cerr << "Server listening error" << endl;
		exit(1);
	}
	
	return openSocket;
}

int serverNegotiateClientConnection(const char* host, const char* port) {
	//create address structs
	struct addrinfo hints, *res;
	
	//initializing address struct
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	//parse address 
	int addressStatus = getaddrinfo(host, port, &hints, &res);
	if (addressStatus != 0) {
		cout << "Server cannot get client address info" << endl;
		return -1;
	}
	
	//loop through response pointer
	int openSocket;
	struct addrinfo* current;
	for (current = res; current != NULL; current = current -> ai_next) {
		//create socket for client
		openSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (openSocket < 0) {
			cerr << "Client cannot open socket" << endl;
			continue;
		}
		
		//establish connection
		int connectStatus = connect(openSocket, current->ai_addr, current->ai_addrlen);
		if (connectStatus < 0) {
			close(openSocket);
			cerr << "Client cannot connect" << endl;
		}
		
		break;
	}
	
	//if cannot bind
	if (current == NULL) {
		cerr << "Client cannot bind socket" << endl;
		return -2;
	}
	
	//free unneeded memory
	free(res);
	
	return openSocket;
}

int serverRetriveRemoteData(int remote, string& data) {
	return 0;
}
