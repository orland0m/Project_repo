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
#include "boost/date_time/time_zone_base.hpp"
#include <boost/date_time.hpp>

using namespace boost::posix_time;

using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace boost::gregorian;
using namespace boost::local_time;
using namespace std;

#define BUFFER_SIZE 1024


string getData(string);
void putData(string,string);
string GetErrorPage(int);

/**
	Tests whether a date has expired
*/
int isExpired(string date){/*
	int expired = 1;
	try{
		ptime pt;
		time_input_facet format = new time_input_facet("%a, %d %b %Y %H:%M:%S %Z")
		const locale loc = locale(locale::classic(),format);
		
		istringstream is(date);
        is.imbue(loc);
        is >> pt;
        if(pt != ptime())
		
		delete format;
	}catch(...){
		expired = 1;
	}
	return expired;*/
	return 0;
}


/**
	Top level function to get response from cache
*/
string GetFromCache(HttpRequest * request, int returnExpired){
	string expires = "";
	string data = getData(request->GetHost()+request->GetPath());
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
	try{
		cout<< getpid() << ": Caching" << endl;
		response -> ParseResponse(buffer.c_str(), buffer.length());
		int code = atoi(response->GetStatusCode().c_str());
		cout<< getpid() << ": cache: Processing code " << code << endl;
		int twoH = 1; // it is used to use 200's save feature. Its purpose is to update the Expires date
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
		path path_name = path("cache/"+url);
		stringstream ss;
		ss << path_name.parent_path();
		string s_parent = ss.str();
		create_directories(s_parent);
		system(("touch "+path_name.native_file_string()).c_str());
		file_lock f_lock(path_name.native_file_string().c_str());
		sharable_lock<file_lock> sh_lock(f_lock);
		size_t filedesc = open(path_name.native_file_string().c_str(), O_WRONLY | O_TRUNC);
    	if(filedesc < 0){
    		cout << "Error writing" << endl;
       		return;
       	}
		write(filedesc,data.c_str(), data.length());
		close(filedesc);
		cout << "Cached: " << path_name.native_file_string() << endl;
	}catch(interprocess_exception e){
		cout << "Write exception: "<< e.what() << endl;
	}catch(...){
		cout << "Exception caught" << endl;
	}
}

string getData(string filename){
	string contents = "";
	try{
		path path_name = path("cache/"+filename);
		if(!exists(path_name)){
			cout << "No file"<<endl;
			goto end;
		}
		file_lock f_lock(path_name.native_file_string().c_str());
		sharable_lock<file_lock> sh_lock(f_lock);
		ifstream in(path_name.native_file_string().c_str(), ios::in | ios::binary);
		if(in){
    		contents.assign((std::istreambuf_iterator<char>(in) ),
                (std::istreambuf_iterator<char>()));
    		in.close();
  		}
	}catch(interprocess_exception e){
		cout << "Read exception: "<< e.what() << endl;
	}catch(...){
		contents = "";
	}
	end:
	return contents;
}