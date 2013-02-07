#ifndef _WEB_REQUEST_
#define _WEB_REQUEST_

#include "http-response.h"
#include "http-request.h"

/**
 * GetRequestedPage's contract:
 * IN: A properly filled http request. 
 * 		A previously set up socket
 * OUT: An http response. That response is ready to be sent to the client.
 */
string GetFromRemoteServer(HttpRequest *, int);

#endif
