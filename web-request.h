#ifndef _CACHE_
#define _CACHE_

#include "http-response.h"
#include "http-request.h"

/**
 * GetRequestedPage's contract:
 * IN: A properly filled http request. 
 * 		A previously set up socket
 * OUT: An http response. That response is ready to be sent to the client.
 */
HttpResponse * GetRequestedPage(HttpRequest *, int);

#endif
