#ifndef _RECEIVE_TIMEOUT_
#define _RECEIVE_TIMEOUT_

/**
	recvtimeout's contract:
	IN: int:socket, char*:data, int:length
	OUT: Number of bytes sent,
	-1 error,
	-2 timeout of 30 seconds
*/
int recvtimeout(int, char *, int);

#endif