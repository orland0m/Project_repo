#include "http-request.h"
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
	while (true) {
		//buffer used for storing incoming data
		char buffer[BUFFER_SIZE];
		
		//gets data from remote end
		int received = recv(remote, buffer, sizeof(remote), 0);
		if (received < 0) {
			cerr << "Retrieve remote data receive error" << endl;
			return -1;
		} else if (received == 0) { // if nothing received, end connection
			break;
		}
		//adds newly retrieved data to old data
		data.append(buffer, received);
	}
	return 0;
}

int serverClientConnection(int client, pthread_mutex_t *mutex) {
	//client buffer
	//string is used because it is dynamically resizable
	string buffer = "";
	
	//loop until HTTP end is found in buffer
	while (buffer.find("\r\n\r\n") < 0) {
		char receiveBuffer[BUFFER_SIZE];
		
		//receive data and place in buffer
		if (recv(client, receiveBuffer, sizeof(receiveBuffer), 0) < 0) {
			cerr << "Client connection receive error" << endl;
			return -1;
		}
		//append receiving buffer to total buffer
		buffer.append(receiveBuffer);
	}
	
	//read client request
	HttpRequest clientRequest;
	try {
		clientRequest.ParseRequest(buffer.c_str(), buffer.length());
	} catch (ParseException requestException) { //handle special cases
		cerr << "Exception in client connection: " << requestException.what() << endl;
		//send HTTP 1.0 for backwards compatibility
		//1.0 and 1.1 shouldn't matter when there are errors anyway
		string response = "HTTP/1.0 ";
		
		//if the GET request is messed up
		if (strcmp(requestException.what(), "Request is not GET") != 0) {
			response += "400 Bad Request\r\n\r\n";
		} else { //it is not a GET request 
			response += "501 Not Implemented\r\n\r\n";
		}
		
		//send the error response to client
		if (send(client, response.c_str(), response.length(), 0) < 0) {
			cerr << "Client Connection send error" << endl;
		}
	}
	
	//if persistent connection is not working, enable this
	//clientRequest.ModifyHeader("Connection", "close");
	
	//create new request
	size_t length = clientRequest.GetTotalLength() + 1; //extra for 0 byte
	char* request = (char*) malloc(length);
	clientRequest.FormatRequest(request);
	
	//if host is not in request already, find host in header or request
	string host;
	if (clientRequest.GetHost().length() == 0) {
		host = clientRequest.FindHeader("Host");
	} else {
		host = clientRequest.GetHost();
	}
	
	//url of file to get
	string path = host + clientRequest.GetPath();
	
	//response to send to client
	string response;
	
	//find in local cache
	if (false) { //if it exists in cache, false is only temporary placeholder
		return -1;
	} else { //else it is not in cache, get from remote
		//establish connection
		//const char* requires conversion
		int remote = serverNegotiateClientConnection(host.c_str(), LISTENING_PORT);
		if (remote < 0) { //client connection error
			cerr << "Client connection cannot connect to remote host" << endl;
			free(request);
			return -1;
		}

		//send request to remote
		if (send(remote, request, length, 0) == -1) {
			cerr << "Client connection cannot send request" << endl;
			free(request);
			close(remote);
			return -1;
		}
		
		// put received response into string
		if (serverRetrieveRemoteData(remote, response) != 0) {
			free(request);
			close(remote);
			return -1;
		}
		
		//should probably implement add to cache function here

		close(remote);
	}
	
	//send everything back to client
	if (send(client, response.c_str(), response.length(), 0) < 0) {
		cerr << "Client connection send error" << endl;
		free(request);
		return -1;
	}
	
	//garbage collection
	free(request);
	return 0;
}
