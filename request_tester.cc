#include "http-request.h"
#include <string.h>
#include <iostream>

using namespace std;
int main(){
	const char *buf = "GET http://www.google.com:80/index.html/ HTTP/1.0\r\nContent-Length: \
		80\r\nIf-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT\r\n\r\n";
	HttpRequest * parser = new HttpRequest;
	parser->ParseRequest(buf,strlen(buf));
	char * new_buffer = new char[parser->GetTotalLength()];
	parser->FormatRequest(new_buffer);
	cout << new_buffer << endl;
	delete(new_buffer);
	return 0;
}
