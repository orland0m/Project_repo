/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "cache.h"

using namespace std;

int main (int argc, char *argv[]){
	string data = GetFromCache("www.google.com/index.html");
	if(data!=NULL){
		cout << "Bing! It's in cache..." << endl;
	}else{
		cout << "Oops... This is embarrassing. We don't have that file" << endl;
	}
	return 0;
}
