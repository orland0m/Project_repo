#include <iostream>
#include "boost/date_time/time_zone_base.hpp"
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::local_time;
using namespace std;

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


int main(){
	isExpired("Thu, 01 Dec 1994 16:00:00 GMT");
	isExpired("Sun, 17 Feb 2013 16:00:00 GMT");
}