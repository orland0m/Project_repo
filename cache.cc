/*
	Implementation of the function GetFromCache
*/

#include "http-response.h"
#include "http-request.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <ctime>
#include <clocale>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

#define BUFFER_SIZE 1024
using namespace std;

string getData(string);
int putData(string,string);
void MakeTreeDir(string);
string GetErrorPage(int);
/**
	Used to get the time in seconds of an HTML-date
*/


time_t GMTToSeconds(const char * date){
	const char format[] = "%a, %d %b %Y %H:%M:%S %Z";
	struct tm time;
	bzero(&time, sizeof(struct tm));
	if(strptime(date, format, &time)){
		return mktime(&time);
	}
    cout<< getpid() << ": HTTP-date parse error: " << strerror(errno) << endl;
    return 0;
}

/**
	Tests whether a date has expired
*/
int isExpired(string date){
	time_t rawtime;
	struct tm * ptm;
	time(&rawtime);
	ptm = gmtime(&rawtime);
	time_t now = mktime(ptm);
	time_t docs = GMTToSeconds(date.c_str());
	return now>docs;
}


/**
	Top level function to get response from cache
*/
string GetFromCache(HttpRequest * request, int returnExpired){
	string expires = "";
	string data = getData("cache/"+request->GetHost()+request->GetPath());
	int dataLength = data.length();
	if(dataLength>1){
		try{
			cout<< getpid() << ": Data in cache..." << endl;
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str(),dataLength);
			expires = response -> FindHeader("Expires")+"";
			cout<< getpid() << ": Expires: " << expires << endl;
			if(!returnExpired && isExpired(expires)){
				cout<< getpid() << ": Expired!" << endl;
				if(string("").compare(request->FindHeader("If-Modified-Since")) == 0){
					cout<< getpid() << ": Adding If-Modified-Since header: "<< expires << endl;
					request->AddHeader("If-Modified-Since", expires);
				}
				delete response;
			}else{
				cout<< getpid() << ": Returning data from cache" << endl;
				delete response;
    			return data;
    		}
  		}catch (...){
  		}
	}else{
		cout<< getpid() << ": Data not in cache" << endl;
	}
	return "";
}


string cleanCharacters(char * header){
	string tmp = "";
	for(int i=0; ;i++){
		if(header[i]=='\r'&&header[i+1]=='\n'&&header[i+2]=='\r'&&header[i+3]=='\n'){
			tmp += "\r\n\r\n";
			break;
		}
		tmp += header[i];
	}
	return tmp;
}


/**
	Top level function to save response to cache
*/
string SaveToCache(string buffer, string url){
	HttpResponse * response = new HttpResponse;
	response -> ParseResponse(buffer.c_str(), buffer.length());
	int code = atoi(response->GetStatusCode().c_str());
	cout<< getpid() << ": cache: Processing code " << code << endl;
	int twoH = 1; // it is used to use 200's save feature. Its purpose is to update the Expires date
	try{
		switch(code){
			case 304: {
				twoH = 0;
				string cacheData = getData("cache/"+url); 
				int cacheDataLength = cacheData.length();
				if(cacheDataLength>0){
					buffer = cacheData;
					string expDate = response -> FindHeader("Expires");				
					if(!isExpired(expDate)){
						delete(response);
						response = new HttpResponse;
						response -> ParseResponse(buffer.c_str(),buffer.length());
						string strTmp = buffer.substr(response -> GetTotalLength(), buffer.length());;
						cout<< getpid() << ": Updating Expires date: " << expDate << endl;
						response -> ModifyHeader("Expires",expDate);
						char * header = new char[response->GetTotalLength()];
						if(header){
							memset(header,'\0',response->GetTotalLength());
							response -> FormatResponse(header);
							buffer = cleanCharacters(header)+strTmp;
							twoH = 1;
							delete(header);
						}
					}
				}else{
					cout<< getpid() << ": 304 but didn't find data in cache" << endl;
				}
			}
			case 200: {
				if(twoH && !isExpired(response -> FindHeader("Expires"))){
					if(putData(url, buffer)){
						cout << "Response saved to cache" << endl;
					}else{
						cout << "Failed saving response to file" << endl;
					}
				}else{
					cout<< getpid() << ": Document expired: "<< response -> FindHeader("Expires") <<". Not saved!" << endl;
				}
			}
		}
	}catch(...){
		buffer = GetErrorPage(500);
	}
	delete(response);
	return buffer;
}


/**
	This function returns an http error message
*/
string GetErrorPage(int errorNumber){
	return "HTTP/1.0 500 Internal Proxy Error\r\n\r\n";
}


/**
	This function gets the base directory of a string
	IN: cache/a/
	OUT: cache/
	OUT ARG: a/
*/
string GetBaseDir(string& s){
	char const * tmp = s.c_str();
	string r = "";
	for(unsigned int i=0; i<s.length(); i++){
		if(tmp[i]=='/'){ 
			s = string(s,i+1,s.length());
			return r+"/";
		}
		r += tmp[i];
	}
	return "";
}


/**
	Create a directory tree
	Can be any path, relative or absolute. It will only try, it doesn't throw erros
	if it doesn't have permission or the directory is already there.
	IN: "cache/a/b/c"
	SIDE EFECT: Will create directory cache/a/b/
	NOTE: A final slash is needed to create cache/a/b/c/
	IT will go down to the last leave, even if there are errors.
	This because it can be called when the directories are already there.
*/
void MakeTreeDir(string missing){
	string done = "";
	string tmp = "";
	while((tmp = GetBaseDir(missing)).compare("")!=0){
		done+=tmp;
		mkdir(done.c_str(), 0777);
	}
}

/**
	Write/Get data from/to files
*/

int putData(string path, string data){
	create_directories(path);
}

string getData(string filename){
	
}