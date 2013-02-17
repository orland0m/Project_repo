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
		ptime pt;
		const locale loc = locale(locale::classic(),new time_input_facet("%a, %d %b %Y %H:%M:%S %Z"));
		
		istringstream is(date);
        is.imbue(loc);
        is >> pt;
        
        ptime curr_time(second_clock::local_time());
        cout << "Current time: " << curr_time << endl;
        cout << "File time: " << pt << endl;
		if(cur_time>pt){
			cout << "Current time is greater" << endl;
		}else{
			cout << "File time is greater" << endl;
		}
		//delete format;
	}catch(...){
		expired = 1;
	}
	return expired;
}


int main(){
	isExpired("Thu, 01 Dec 1994 16:00:00 GMT");
	isExpired("Sun, 17 Feb 2013 16:00:00 GMT");
}