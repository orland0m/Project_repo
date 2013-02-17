/*
 * Functionality to retrieve/save files from/to cache
 */

#ifndef _CACHE_
#define _CACHE_

//C++ libraries
#include <iostream>

// User defined libraries
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
 *				to the request in the first argument
 */
string GetFromCache(HttpRequest *, int);


/**
 * GetErrorPage's contract:
 * IN: string. A message to add to a 500 type error
 * OUT: A 500 html error message with the message appended to it.
 */

string GetErrorPage(string);

/**
 * SaveToCache's contract:
 * IN: an http response, the name of the file (server+path)
 * OUT: The recently stored request. Or if it was a 304, the file that was already in cache,
 		if it the file doesn't need to be cached then it will return the first argument.
 * NOTE: Ideally you pass all responses to this function and it will determine what to do
 *			with the received data and what data has to be sent to the client
 * SIDE EFFECTS: If the response has a non expired date it will be stored in cache/filename
 */
string SaveToCache(string, string);

#endif