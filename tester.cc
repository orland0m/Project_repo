#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::filesystem;
int main(){
	create_directories("boost/tester/abc.txt");
}