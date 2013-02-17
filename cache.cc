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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

using namespace boost::filesystem;
using namespace boost::interprocess;

#define BUFFER_SIZE 1024
using namespace std;

string getData(string);
void putData(string,string);
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
				string cacheData = getData(url); 
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
					cout << getpid() << ": Saving to cache" << endl;
					putData(url, buffer);
				}else{
					cout << getpid() << ": Document expired: "<< response -> FindHeader("Expires") <<". Not saved!" << endl;
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
	Write/Get data from/to files
*/

void putData(string url, string data){
	try{
		file_lock f_lock("cache/"+url);
		sharable_lock<file_lock> sh_lock(f_lock);
		path path_name = path("cache/"+url);
		create_directories(path_name.parent_path());
		ofstream file;
		file.open(("cache/"+url).c_str(),ios::trunc);
		file << data;
		file.close();
		file.flush();
	}catch(...){
	}
}

string getData(string filename){
	string contents = "";
	try{
		file_lock f_lock("cache/"+filename);
		sharable_lock<file_lock> sh_lock(f_lock);
		ifstream in(filename.c_str(), ios::in | ios::binary);
		if(in){
    		in.seekg(0, ios::end);
    		int pointer = in.tellg();
    		if(pointer<1) return "";
    		contents.resize(pointer);
    		in.seekg(0, ios::beg);
    		in.read(&contents[0], contents.size());
    		in.close();
  		}
	}catch(...){
		contents = "";
	}
	return contents;
}