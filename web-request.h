#ifndef _CACHE_
#define _CACHE_


/**
 * GetRequestedPage's contract:
 * IN: A char array. This array contains the data as it was received from the client.
 * OUT: A char array. This array is the data that should be sent to the client.
 */
const char * GetRequestedPage(const char *);

#endif
