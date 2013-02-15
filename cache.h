/*
 * Functionality to retrieve/save files from/to cache
 */

#ifndef _CACHE_
#define _CACHE_

#include <iostream>
#include "http-request.h"
#include "http-response.h"
using namespace std;

/**
 * GetFromCache's contract:
 * IN: An HTTP request. An integer [0,1] a flag to ignore expires header
 * OUT: The HTTP Response that was stored in cache. Empty string if it was expired or
 *		It didn't exist.
 * NOTE: If the file has a badly formated 'Expires' date it's going to be
 *		treated as an expired file. However, that should never be the case, because SaveToCache
 *		would never allow it.
 * SIDE EFFECTS: If the file was expired, and the request doesn't have an If-Modified-Since
 * 				header, this method will add the expiration date as an If-Modified-Since header
 *				to the request
 */
string GetFromCache(HttpRequest *, int, pthread_mutex_t *);


/**
 * GetErrorPage's contract:
 * IN: Integer. That integer is to be a valid http error code that is already stored
 * 		in cache.
 * OUT: The html error page corresponding to that error code.
 * NOTES: Not fully implemented. Right now it returns a google 404, or 500 if there was a
 *			problem reading.
 * PRECONDITIONS: when requesting an error page, e.g. 404, there has to be an http response
 * 				stored in cache/stderr/<ERROR_NUMBER>.html, otherwise it will return 
 *				a 500 response error loaded from memory
 */

string GetErrorPage(int);

/**
 * SaveToCache's contract:
 * IN: an http response, the name of the file (server+path)
 * OUT: The recently stored request. Or if it was a 304, the file that was already in cache,
 		if it the file doesn't need to be cached then it will return the first argument.
 * NOTE: Ideally you pass all responses to this function and it will determine what to do
 *			with the received data and what data has to be sent to the client
 * SIDE EFFECTS: If the response has non expired date it will be stored in cache/filename
 */
string SaveToCache(string, string, pthread_mutex_t *mutex);

#endif