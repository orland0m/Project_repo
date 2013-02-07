/*
	Implementation of the function GetFromCache
*/

#include "http-response.h"
#include "http-request.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <ctime>
#include <clocale>
using namespace std;
string default_response = "HTTP/1.1 500 Internal Proxy Error\r\n\r\n";

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
	time(&rawtime);
	ptm = gmtime ( &rawtime );
	time_t now = mktime(ptm);
	time_t docs = GMTToSeconds(date.c_str());
	return now>docs;
}

string GetFromCache(HttpRequest * request, int returnExpired){
	string expires = "";
	string data = getData("cache/"+request->GetHost()+request->GetPath()); 
	int dataLength = data.length();
	if(dataLength>1){
		try{
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str(),dataLength);
			if(!returnExpired && isExpired(expires = response -> FindHeader("Expires"))){
				if(string("").compare(request->FindHeader("If-Modified-Since")) == 0){
					request->AddHeader("If-Modified-Since", expires);
				}
				delete response;
			}else{
    			return data;
    		}
  		}catch (int e){
  		}
	}
	return NULL;
}


string SaveToCache(string buffer, string url){
	HttpResponse * response = new HttpResponse;
	response -> ParseResponse(buffer.c_str(), buffer.length());
	int code = atoi(response->GetStatusCode().c_str());
	int twoH = 1; // it is used to use 200's save feature. Its purpose is to update the Expires date
	switch(code){
		case 304: {
			twoH = 0;
			string data = getData(url); 
			int dataLength = data.length();
			if(dataLength>=1){
				string expDate = response -> FindHeader("Expires");
				int expired = isExpired(expDate);
				response = new HttpResponse;
				response -> ParseResponse(data.c_str(),dataLength);
				if(!expired){
					response -> ModifyHeader("Expires",expDate);
					twoH = 1;
				}
			}
		}
		case 200: {
			if(twoH && !isExpired(response -> FindHeader("Expires"))){
				size_t length = response -> GetTotalLength();
				char * data = new char[length];
				if(data){
					data[0] = '\0';
					int dataStart = buffer.length() - length;
					buffer = buffer.substr(dataStart,buffer.length());
					response -> FormatResponse(data);
					buffer = string(data) + buffer;
					ofstream file;
					file.open(("cache/"+url).c_str(),ios::trunc);
					file << buffer;
					delete(data);
					file.close();
				}
			}
		}
	}
	return buffer;
}

string getData(string filename){
  ifstream in(filename.c_str(), ios::in | ios::binary);
  if (in){
    string contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  return "";
}


string GetErrorPage(int errorNumber){
	string path = "cache/stderr/404.html";
	switch(errorNumber){
		default:
			path = "cache/stderr/404.html";
	}
	string data = getData(path);
	if(data.length()<1){
		data = "HTTP/1.1 500 Internal Proxy Error\r\n\r\n";
	}
	return data;
}