/*
 * Functionality to retrieve/save files from/to cache
 */

#ifndef _CACHE_
#define _CACHE_

#include <iostream>
#include "http-request.h"
using namespace std;

/**
 * GetFromCache's contract:
 * IN: An HTTP request. An out string. an integer [0,1] a flag to ignore expires header
 * OUT: The HTTP Response that was stored in cache. NULL if it was expired or
 *		It didn't exist.
 * NOTE: If the file has a badly formated 'Expires' date it's going to be
 *		treated as an expired file.
 * SIDE EFFECTS: If the file was expired, and the request doesn't have a If-Modified-Since
 * 				header, this method will add the expiration date as an If-Modified-Since header
 */
HttpResponse * GetFromCache(HttpRequest *, int);


/**
 * GetErrorPage's contract:
 * IN: Integer. That integer is to be a valid http error code that is already stored
 * 		in cache.
 * OUT: The html error page corresponding to that error code.
 */

HttpResponse * GetErrorPage(int);

/**
 * SaveToCache's contract:
 * IN: an http response, the name of the file (server+path)
 * OUT: The recently stored request, or if it was a 304, the file that was already in cache
 * NOTE: Ideally you pass all responses to this function and it will return the response data
 *		ready to be sent to the client
 * 		If the file has non expired date it will be stored at cache/filename
 */
HttpResponse * SaveToCache(HttpResponse *, string);

#endif