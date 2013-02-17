// C++ libraries
#include <iostream>

// C libraries
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// Namespaces
using namespace std;


// Same function as recv; with a timeout limit (param timeout)

// Retval: -1: error, -2: timeout

int recvtimeout(int s, char *buf, int len){
	int timeout = 30; // default 30 second timeout
    fd_set fds;
    int n;
    struct timeval tv;
    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    // set up the struct timeval for the timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    // wait until timeout or data received
    n = select(s+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error
    // data must be here, so do a normal recv()
    return recv(s, buf, len, 0);
}