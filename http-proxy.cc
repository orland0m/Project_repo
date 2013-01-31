/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "web-request.h"

using namespace std;

int main (int argc, char *argv[]){
	string file = "cache/www.google.com/index.html"; // right now it works with files only
	const char * data = GetRequestedPage(file.c_str());
	if(data){
		cout << "Bingo! your file is in: " << file << endl;
	}else{
		cout << "Oops... This is embarrassing. We don't have that file yet" << endl;
	}
	return 0;
}
