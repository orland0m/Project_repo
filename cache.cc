/*
	Implementation of the function GetFromCache
*/

#include "http-response.h"
#include "http-request.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <ctime>
#include <clocale>
#include <sys/stat.h>
using namespace std;
string default_response = "HTTP/1.1 500 Internal Proxy Error\r\n\r\n";

string getData(string);
void MakeTreeDir(string, string);

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
	Top level function to get response from cache
*/
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
					cout << "Adding If-Modified-Since header" << endl;
					request->AddHeader("If-Modified-Since", expires);
				}
				delete response;
			}else{
				cout << "Returning retrieved data from cache" << endl;
    			return data;
    		}
  		}catch (...){
  		}
	}else{
		cout << "Data not in cache" << endl;
	}
	return "";
}



/**
	Top level function to save response to cache
*/
string SaveToCache(string buffer, string url){
	HttpResponse * response = new HttpResponse;
	response -> ParseResponse(buffer.c_str(), buffer.length());
	int code = atoi(response->GetStatusCode().c_str());
	cout << "Processing code " << code << endl;
	int twoH = 1; // it is used to use 200's save feature. Its purpose is to update the Expires date
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
					cout << "Updating Expires date..." << endl;
					response -> ModifyHeader("Expires",expDate);
					char * charTmp = new char[response->GetTotalLength()];
					if(charTmp){
						memset(charTmp,'\0',response->GetTotalLength());
						response -> FormatResponse(charTmp);
						buffer = string(charTmp)+"<!--304 Proxy server comment-->" + strTmp;
						twoH = 1;
						delete(charTmp);
					}
				}
			}else{
				cout << "304 but didn't find data in cache" << endl;
			}
		}
		case 200: {
			if(twoH && !isExpired(response -> FindHeader("Expires"))){
				ofstream file;
				file.open(("cache/"+url).c_str(),ios::trunc);
				file << buffer;
				cout << "Saved to cache:  "<< url << endl;
				file.close();
			}else{
				cout "Document expired... not saved!" << endl;
			}
		}
	}
	delete(response);
	return buffer;
}


/**
	This function returns an http error message
*/
string GetErrorPage(int errorNumber){
	return "HTTP/1.1 500 Internal Proxy Error\r\n\r\n";
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
	Recursive function to create a directory tree
	Can be any path, relative or absolute
	IN: "cache/a/b/c", ""
		"a/b/c", "cache/"
	SIDE EFECT: Will create directory cache/a/b/
	NOTE: A final slash is needed to create cache/a/b/c/
	IT will go down to the last leave, even if there are errors.
	This because it can be called when the directories are already there.
*/
void MakeTreeDir(string missing, string done){
	string tmp = GetBaseDir(missing);
	if(tmp.compare("")==0) return;
	done += tmp;
	mkdir(done.c_str(), 0777);
	MakeTreeDir(missing,done);
}

/**
	This function retrieves data from a file on this, you just provide a file name
	If the file isn't accesible or doesn't exist it returns an empty string
*/

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