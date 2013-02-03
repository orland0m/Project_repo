#include "http-request.h"
#include <iostream>

using namespace std;

int main(){
	const char *buf = "GET http://www.google.com:80/index.html/ HTTP/1.0\r\nContent-Length: \
		80\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\n\r\n";
	HttpRequest * parser = new HttpRequest;
	parser.Parse(buf);
	char * new_buffer = (char *) malloc(parser.GetTotalLength());
	parser.FormatRequest(new_buffer);
	cout << new_buffer << endl;
	return 0;
}
