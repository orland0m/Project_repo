#ifndef _CACHE_
#define _CACHE_


/**
 * GetRequestedPage's contract:
 * IN: A properly filled http request. 
 * 		A previously set up socket (haven't checked how to use sockets
 *		If the socket isn't set up, it creates it and puts the info in the socket.
 * OUT: An http response.
 * 		OUT_ARGUMENT: the integer pointed by the second argument will be the file
 *					file descriptor for the created socket.
 *				NOTE: If the pointed value is NULL/0 the socket will be created
 */
HttpResponse * GetRequestedPage(HttpRequest *, int *);

#endif
