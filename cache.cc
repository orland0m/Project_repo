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
#include <errno.h>
using namespace std;
string default_response = "HTTP/1.1 500 Internal Proxy Error\r\n\r\n";

string getData(string);
void MakeTreeDir(string);
string GetErrorPage(int);
static const char httpFormat[] = "%a, %d %b %Y %H:%M:%S GMT";
/**
	Used to get the time in seconds of an HTML-date
*/
/* time_t GMTToSeconds(const char *date){
    char buffer[50];
    memset(buffer,'\0',50);
	struct tm tm;
	bzero(&tm, sizeof(tm));
	if(strptime(date, httpFormat, &tm)){
		time_t timePST = mktime(&tm);
		&tm = gmtime(&timePST);
		strftime(buffer,50,httpFormat,&tm);
		cout << "File Not Parsed: " << date << endl;
		cout << "File Parsed: " << buffer << endl;
		return mktime(&tm);
	}
    cout << "HTTP-date parse error: " << strerror(errno) << endl;
    return 0;
} */
time_t GMTToSeconds(const char *date){
    char buffer[50];
    memset(buffer,'\0',50);
	struct tm * time = new struct tm;
	bzero(time, sizeof(struct tm));
	if(strptime(date, httpFormat, time)){
		time_t timePST = mktime(time);
		time = gmtime(&timePST);
		strftime(buffer,50,httpFormat,time);
		cout << "File Not Parsed: " << date << endl;
		cout << "File Parsed: " << buffer << endl;
		return mktime(time);
	}
    cout << "HTTP-date parse error: " << strerror(errno) << endl;
    return 0;
}

/**
	Tests whether a date has expired
*/
int isExpired(string date){
	time_t rawtime;
	char buffer[50];
	memset(buffer,'\0',50);
	struct tm * ptm;
	time(&rawtime);
	ptm = gmtime(&rawtime);
	time_t now = mktime(ptm);
	strftime(buffer,50,httpFormat,ptm);
	cout << "Local date: " << buffer << endl;
	time_t docs = GMTToSeconds(date.c_str());
	return now>docs;
}


/**
	Top level function to get response from cache
*/
string GetFromCache(HttpRequest * request, int returnExpired, pthread_mutex_t *mutex){
	string expires = "";
	pthread_mutex_lock(mutex);
	string data = getData("cache/"+request->GetHost()+request->GetPath()); 
	pthread_mutex_unlock(mutex);
	int dataLength = data.length();
	if(dataLength>1){
		try{
			cout << "Data in cache..." << endl;
			HttpResponse * response = new HttpResponse;
			response -> ParseResponse(data.c_str(),dataLength);
			expires = response -> FindHeader("Expires");
			cout << "Expires: " << expires << endl;
			if(!returnExpired && isExpired(expires)){
				cout << "Expired!" << endl;
				if(string("").compare(request->FindHeader("If-Modified-Since")) == 0){
					cout << "Adding If-Modified-Since header: "<< expires << endl;
					request->AddHeader("If-Modified-Since", expires);
				}
				delete response;
			}else{
				cout << "Returning data from cache" << endl;
				delete response;
    			return data;
    		}
  		}catch (...){
  		}
	}else{
		cout << "Data not in cache" << endl;
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
string SaveToCache(string buffer, string url, pthread_mutex_t *mutex){
	HttpResponse * response = new HttpResponse;
	response -> ParseResponse(buffer.c_str(), buffer.length());
	int code = atoi(response->GetStatusCode().c_str());
	cout << "Processing code " << code << endl;
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
						cout << "Updating Expires date: " << expDate << endl;
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
					cout << "304 but didn't find data in cache" << endl;
				}
			}
			case 200: {
				if(twoH && !isExpired(response -> FindHeader("Expires"))){
					cout << "Caching response..." << endl;
					pthread_mutex_lock(mutex); // lock writing
					MakeTreeDir("cache/"+url);
					ofstream file;
					file.open(("cache/"+url).c_str(),ios::trunc);
					file << buffer;
					cout << "Saved to cache:  "<< url << endl;
					file.close();
					pthread_mutex_unlock(mutex); // unlock
				}else{
					cout << "Document expired: "<< response -> FindHeader("Expires") <<". Not saved!" << endl;
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