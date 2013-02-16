#include <unistd.h>
#include <iostream>

using namespace std;
void console_print(string str){
	cout << getpid() <<": " <<  str << endl;
}