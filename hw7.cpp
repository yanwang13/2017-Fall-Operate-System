#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <sys/time.h>
#include <strings.h>
#include <string>
#define SMALL_FILE_SIZE 1024*64
#define LARGE_FILE_SIZE 1024*1024*50
#define FILE_MAX 1400
#define PATH_LEN 1200

using namespace std;

int main(int argc, char** argv){
	struct timeval start, end;
	char* large_buffer;
	char* small_buffer;
	
	if(argc!=2){
		cout << "Usage: ./hw7 <path>\n";
		exit(1);
	}

	gettimeofday(&start, NULL);

	small_buffer = new char[SMALL_FILE_SIZE];
	large_buffer = new char[LARGE_FILE_SIZE];

	ofstream file;

	char filepath[PATH_LEN];
	char cmd[1500];
	for(int i=0;i<FILE_MAX;++i){//create small files
		bzero(filepath, PATH_LEN);
		sprintf(filepath, "%s/%d.txt",  argv[1], i);
		file.open(filepath, ios::out);
		//file.write(small_buffer, sizeof(small_buffer));
		file.write(small_buffer, SMALL_FILE_SIZE*sizeof(char));
		file.close();
	}
	sync();
	for(int i=0;i<FILE_MAX;i+=2){//remove even files to create holes
		bzero(filepath, PATH_LEN);
		sprintf(filepath, "%s/%d.txt",  argv[1], i);
		remove(filepath);
	}

	bzero(filepath, PATH_LEN);
	sprintf(filepath, "%s/largefile.txt",  argv[1]);
	file.open(filepath, ios::out);
	file.write(large_buffer, LARGE_FILE_SIZE*sizeof(char));
	file.close();
	sync();
	sprintf(cmd, "filefrag -v %s", filepath);
	system(cmd);
	gettimeofday(&end, NULL);
	if((end.tv_usec-=start.tv_usec)<0){
		end.tv_usec += 1000000;
		--end.tv_sec;
	}
	end.tv_sec -= start.tv_sec;

	cout<< "time: " << end.tv_sec+(end.tv_usec/1000000.0) << " sec\n";


	for(int i=1;i<FILE_MAX;i+=2){//remove odd files
		bzero(filepath, PATH_LEN);
		sprintf(filepath, "%s/%d.txt",  argv[1], i);
		remove(filepath);
	}

	delete [] large_buffer;
	delete [] small_buffer;

	return 0;
}
