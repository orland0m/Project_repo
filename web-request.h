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
 *		OUT ARG: If the socket couldn't be used, it's set to -1
 */
string GetFromRemoteServer(HttpRequest *, int&);

#endif
