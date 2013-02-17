// C libraries 
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

// C++ libraries
// --DATE
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/date_time/time_zone_base.hpp"
#include <boost/date_time.hpp>

// --File functionality
#include <boost/filesystem.hpp>

// --File locking
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>

// --General purpose
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <clocale>

// User defined libraries
#include "http-response.h"
#include "http-request.h"

// Constant
#define BUFFER_SIZE 1024

// namespaces for convenience
using namespace boost::posix_time;
using namespace boost::filesystem;
using namespace boost::interprocess;
using namespace boost::gregorian;
using namespace boost::local_time;
using namespace std;

/*
	getData's contract:
	IN: string:filename
	OUT: string:file contents
	NOTE: If the file doesn't exist it returns empty string
	NOTE: It locks the file it's accessing
*/
string getData(string);
/*
	putData's contract:
	IN: string: path, string: data
	SIDE EFFECT: If creates the whole directory tree for path, and then saves data to
	the file specified by path
*/
void putData(string,string);

/* Prototype. Same as cache.h:GetErrorPage */
string GetErrorPage(string);

/**
	Tests whether a date has expired
*/
int isExpired(string date){
	int expired = 1;
	try{
		// Create locale
		ptime pt;
		const locale loc = locale(locale::classic(),new time_input_facet("%a, %d %b %Y %H:%M:%S %Z"));
		// Create string stream
		istringstream is(date);
		// Format string with custom locale
        is.imbue(loc);
        is >> pt;
        //Get current time
        ptime curr_time(second_clock::local_time());
		if(curr_time>pt){
			expired = 1;
		}else{
			expired = 0;
		}
	}catch(...){
		expired = 1;
	}
	return expired;
}


/**
	Top level function to get response from cache. Same as cache.h:GetFromCache
*/
string GetFromCache(HttpRequest * request, int returnExpired){
	string expires = "";
	string data = getData(request->GetHost()+request->GetPath()); // Get from disk
	int dataLength = data.length();
	if(dataLength>1){ // If the data exists
		try{
			cout<< getpid() << ": Data in cache..." << endl;
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str(),dataLength);
			expires = response -> FindHeader("Expires")+"";
			cout<< getpid() << ": Expires: " << expires << endl;
			if(!returnExpired && isExpired(expires)){ // if the content is expired, and the user wants non-expired content
				cout<< getpid() << ": Expired!" << endl;
				if(string("").compare(request->FindHeader("If-Modified-Since")) == 0){ // add expires header
					cout<< getpid() << ": Adding If-Modified-Since header: "<< expires << endl;
					request->AddHeader("If-Modified-Since", expires);
				}
				delete response;
			}else{ // return data from cache
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

/**
	This function cleans garbage added by ParseResponse
*/
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
	Top level function to save response to cache. Same as cache.h:SaveToCache
*/
string SaveToCache(string buffer, string url){
	HttpResponse * response = new HttpResponse;
	try{
		cout<< getpid() << ": Caching" << endl;
		response -> ParseResponse(buffer.c_str(), buffer.length());
		int code = atoi(response->GetStatusCode().c_str());
		cout<< getpid() << ": cache: Processing code " << code << endl;
		int twoH = 1; // it is used to use 200's save feature. Its purpose is to update the Expires date
		switch(code){ // process code
			case 304: {
				twoH = 0;
				string cacheData = getData(url); 
				int cacheDataLength = cacheData.length();
				if(cacheDataLength>0){ // if the file was cached
					buffer = cacheData;
					string expDate = response -> FindHeader("Expires");				
					if(!isExpired(expDate)){ // if the recently arrived content isn't expired
						delete(response);
						response = new HttpResponse;
						response -> ParseResponse(buffer.c_str(),buffer.length());
						string strTmp = buffer.substr(response -> GetTotalLength(), buffer.length());;
						cout<< getpid() << ": Updating Expires date: " << expDate << endl;
						response -> ModifyHeader("Expires",expDate); // update expires date in header
						char * header = new char[response->GetTotalLength()];
						if(header){
							memset(header,'\0',response->GetTotalLength());
							response -> FormatResponse(header);
							buffer = cleanCharacters(header)+strTmp;
							twoH = 1; // use 200's save functionality
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
					putData(url, buffer); // try to save
				}else{
					cout << getpid() << ": Document expired, not saved: "<< response -> FindHeader("Expires") << endl;
				}
			}
		}
	}catch(...){
		buffer = GetErrorPage("There was an issue parsing the web server response");
	}
	delete(response);
	return buffer;
}


/**
	This function returns an http error message
*/
string GetErrorPage(string message){
	return "HTTP/1.0 500 Internal Proxy Error\r\n\r\n"+message;
}




/**
	Write/Get data from/to files
*/

void putData(string url, string data){
	try{
		// Get directory tree
		path path_name = path("cache/"+url);
		stringstream ss;
		ss << path_name.parent_path();
		string s_parent = ss.str();
		// create directory tree
		create_directories(s_parent);
		// Touch file, wouldn't affect previously written data
		system(("touch "+path_name.native_file_string()).c_str());
		//Lock file
		file_lock f_lock(path_name.native_file_string().c_str());
		sharable_lock<file_lock> sh_lock(f_lock);
		// Trunk file
		size_t filedesc = open(path_name.native_file_string().c_str(), O_WRONLY | O_TRUNC);
    	if(filedesc < 0){
    		cout << "Error writing" << endl;
       		return;
       	}
       	// put data in disk
		write(filedesc,data.c_str(), data.length());
		close(filedesc);
		cout << "Cached: " << path_name.native_file_string() << endl;
	}catch(...){
	}
}

string getData(string filename){
	string contents = "";
	try{
		path path_name = path("cache/"+filename);
		if(!exists(path_name)){ // if the file doesn't exist
			goto end;
		}
		// lock file
		file_lock f_lock(path_name.native_file_string().c_str());
		sharable_lock<file_lock> sh_lock(f_lock);
		ifstream in(path_name.native_file_string().c_str(), ios::in | ios::binary);
		if(in){
			// put data in string
    		contents.assign((std::istreambuf_iterator<char>(in)), (std::istreambuf_iterator<char>())); 
    		in.close();
  		}
	}catch(...){
		contents = "";
	}
	end:
	return contents;
}