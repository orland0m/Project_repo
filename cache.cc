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
#define MEM_500 (string("HTTP/1.1 500 Internal Proxy Error")).c_str()

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

/**
	First it gets the data from the file, if there was an error the length should be less
	than one. Then it tries parsing to get the expiration date. If it's expired and the
	flag is turned on, then delete the file, else return the data that the file has.
*/
HttpResponse * GetFromCache(HttpRequest * request, int returnExired){
	expires = NULL;
	string data = getData(request->GetHost()+request->GetPath()); 
	int dataLength = data.length();
	if(dataLength>1){
		try{
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str(),dataLength);
			if(!returnExired&&isExpired(expires = response -> FindHeader("Expires"))){
				if((request->FindHeader("If-Modified-Since"))==""){
					request->AddHeader("If-Modified-Since: "+expires);
				}
				delete response;
			}else{
    			return response;
    		}
  		}catch (int e){
  		}
	}
	return NULL;
}


Http-Response * SaveToCache(Http-Response * response, string url){
	int code = stoi(response->GetStatusCode());
	int twoH = 1; // it is used to use 200's save feature. That is when we have a new expiration date
	switch(code){
		case 304:
			twoH = 0;
			string data = getData(url); 
			int dataLength = data.length();
			if(dataLength>1){
				string expDate = response -> FindHeader("Expires");
				int expired = isExpired(expDate);
				response = new HttpResponse;
				response -> ParseResponse(data.c_str(),dataLength);
				if(!expired){
					response -> ModifyHeader("Expires",expDate);
					twoH = 1;
				}
			}
		case 200:
			if(twoH&&!isExpired(response -> FindHeader("Expires"))){
				size_t length = response -> GetTotalLength();
				char * data = (char *) malloc(length);
				if(data){
					data[0] = '\0';
					response -> FormatResponse(data);
					ofstream file;
					file.open("cache/"+url, ios::trunc);
					file << data;
					free(data);
					file.close();
				}
			}
		default:
			return response;
	}
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


HttpResponse * GetErrorPage(int errorNumber){
	string path = "cache/stderr/404.html";
	switch(errorNumber){
		case 404:
		default:
			path = "cache/stderr/404.html";
	}
	string data = getData(path);
	HttpResponse * response = new HttpResponse;;
	if(data.length>1){
		response -> FormatResponse(data.c_str());
	}else{
		response -> FormatResponse(MEM_500);
	}
	return response;
}