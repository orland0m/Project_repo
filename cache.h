/*
 * Functionality to retrieve and save files from/to cache
 */

#ifndef _CACHE_
#define _CACHE_

#include <iostream>
using namespace std;

/**
 * GetFromCache's contract:
 * IN: It receives a string file name. That file exists in disk and is a properly formated
 *		HTTP Response from a web server.
 * 		It also receives an integer that is either 0 or 1.
 * OUT: The file contents as a character array
 * SIDE EFECTS: Side effects are avoided by setting the second argument to 0 (cero)
 *		Side effect: It parses the file to see if it has expired, in which case
 *		deletes the entry from disk, and returns NULL pointer.
 * NOTE: If the file has a badly formated 'Expires' date it's going to be
 *		treated as an expired file.
 */
const char * GetFromCache(string, int);

/**
 * Yet to be implemented
 */
void SaveToCache(char *);

#endif