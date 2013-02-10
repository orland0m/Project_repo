#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main (int argc, char *argv[]){
	string line;
	ifstream myfile ("response.txt");
	if (myfile.is_open()){
    	while ( myfile.good() ){
    		getline(myfile,line);
    		ProcessRequest(line);
    	}
    	myfile.close();
	}else cout << "Unable to open file response.txt";
	return 0;
}