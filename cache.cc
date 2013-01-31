/*
	Implementation of the function GetFromCache
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
    return 0;
}

/**
	Tests whether a date has expired
*/
int isExpired(string date){
	time_t rawtime;
	struct tm * ptm;
	time ( &rawtime );
	ptm = gmtime ( &rawtime );
	time_t now = mktime(ptm);
	time_t docs = GMTToSeconds(date.c_str());
	return now>docs;
}

/**
	First it gets the data from the file, if there was an error the length should be less
	than one. Then it tries parsing to get the expiration date. If it's expired and the
	flag is turned on, then delete the file, else return the data that the file has.
*/
const char * GetFromCache(string file, int o_del){
	string data = getData(file); 
	int dataLength = data.length();
	if(dataLength>1){
		try{
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str(),dataLength);
			if(isExpired(response -> FindHeader("Expires"))){
				if(o_del){
					remove(file);
					cout << "cache.cc: Obsolete file removed: " << file << endl;
				}
				delete response;
			}else{
				delete response;
    			return data.c_str();
    		}
  		}catch (int e){
  		}
	}
	return NULL;
}


string getData(string filename){
  std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
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