/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

//C++ libs
#include <iostream>
#include <string>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "tester.h" /* ProcessFile */

using namespace std;

int main (int argc, char *argv[]){
	std::string s("2001-10-9"); //2001-October-09
    date d(from_simple_string(s));
    std::cout << to_simple_string(d) << std::endl;
	//ProcessFile("request.txt");
	return 0;
}
