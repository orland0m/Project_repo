#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::filesystem;
int main(){
	string path_file = "boost/tester/abc.txt";
	path file = path(path_file);
	cout << file.filename() << endl;
	cout << file.parent_path() << endl;
	//create_directories();
}