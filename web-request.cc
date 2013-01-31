#import "cache.h"
#import <iostream>

using namespace std;

const char * GetRequestedPage(const char * clientRequest){
	string file = clientRequest;
	char * response = GetFromCache(file,1);
	if(response==NULL){
	}
	return response;
}