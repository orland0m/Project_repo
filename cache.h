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
 * IN: An HTTP request. An out string.
 * OUT: 
 * 		Return: The HTTP Response that was stored in cache. NULL if it was expired or
 *				It didn't exist.
 *		Out: The second argument will hold the expiration date of this file.
 * NOTE: If the file has a badly formated 'Expires' date it's going to be
 *		treated as an expired file.
 */
HttpResponse * GetFromCache(HttpRequest *, string &);


/**
 * HttpResponse's contract:
 * IN: Integer. That integer is to be a valid http error code that is already stored
 * 		in cache.
 * OUT: The html error page corresponding to that error code.
 */

HttpResponse * GetErrorPage(int);

/**
 * SaveToCache's contract:
 * IN: an http response, the name of the file (server+path)
 * NOTE: If the file has non expired date it will be stored at cache/filename
 */
void SaveToCache(HttpResponse *, string);

#endif