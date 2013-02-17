#ifndef _WEB_REQUEST_
#define _WEB_REQUEST_

/**
	recvtimeout's contract:
	IN: int:socket, char*:data, int:length
	OUT: Number of bytes sent,
	-1 error,
	-2 timeout of 30 seconds
*/
int recvtimeout(int, char *, int);

#endif