/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include "cache.h"

using namespace std;

int main (int argc, char *argv[]){
	setlocale(LC_ALL,"GMT"); // Leave alone, it's used to change the Locale of this program to GMT
	const char * data = GetFromCache("cache/www.google.com/index.html");
	if(data){
		cout << "Bingo! It's in cache..." << endl;
	}else{
		cout << "Oops... This is embarrassing. We don't have that file" << endl;
	}
	return 0;
}
