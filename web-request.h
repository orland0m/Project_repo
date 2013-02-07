#ifndef _WEB_REQUEST_
#define _WEB_REQUEST_

#include "http-response.h"
#include "http-request.h"
#include <iostream>

using namespace std;

/**
 * GetRequestedPage's contract:
 * IN: A properly filled http request. 
 * 		A previously set up socket
 * OUT: An http response. That response is ready to be sent to the client.
 * NOTES: The second argument is thought to be a socket file descriptor to those keep details
 *		of implementation out of this function.
 *		That socket should be used to send requests and receive web server responses.
 *		I had in mind that a single socket could be used to send several requests to a
 *		single server.
 *		The list of sockets will be then managed by another module. That module could keep
 *		track of all opened connections and reuse them when possible.
 *		Considerations: Since we are going to use multi-threading there might be a problem
 *					if several threads were to use the same socket, because then it would become
 *					a problem demultiplexing data.
 *					Web servers timeout connections after sometime. So the socket might be unusable
 *					when this function tries to send/receive data.
 */
string GetFromRemoteServer(HttpRequest *, int);

#endif
