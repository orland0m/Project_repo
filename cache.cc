/*
	Implementation of the function get from cache
*/

#include "http-response.h"
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <clocale>
using namespace std;
string getData(string);

/**
	Used to get the time in seconds of an HTML-date
*/
time_t GMTToSeconds(const char *date){
	struct tm time_rt={0,0,0,0,0,0,0,0,0};
    if((char *)strptime(date,"%a, %d %b %Y %T GMT",&time_rt)){
    	return mktime(&time_rt);
    }
    return time(NULL)+1; // if there is a badly formed date, 
    					//  we asume the document is not expired
}

/**
	Tests whether a date has expired
*/
int isExpired(char * date){
	time_t now = time(NULL);
	time_t docs = GMTToSeconds(date);
	return now>docs;
}

char * GetFromCache(string file){
	string data = getData(file); 
	size_t dataLength = (size_t)data.length();
	if(dataLength>1){
		try{
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str,dataLength);
			if(isExpired(response -> FindHeader("Expires"))){
				//remove(file);
				delete response;
				cout << "File removed!" << endl;
			}else{
				delete response;
    			return data.c_str;
    		}
  		}catch (int e){
  		}
	}
	return NULL;
}


string getData(string filename){
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in){
    string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  return "";
}