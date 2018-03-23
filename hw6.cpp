#include <dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<string>
#include<iostream>
using namespace std;

bool inode = false;
bool name = false;
bool min_flag = false;
bool max_flag = false;

string filename;
int max_size, min_size;
int inode_num;

void FIND(const char* path);

int main(int argc, char** argv){

	for(int i=2;i<argc;++i){
		if(strcmp(argv[i], "-inode")==0){
			inode_num = atoi(argv[++i]);
			inode = true;
			//cout << "inode_num: " << inode_num << endl;
		}
		if(strcmp(argv[i], "-name")==0){
			filename = argv[++i];
			name = true;
			//cout << "filename: " << filename << endl;
		}
		if(strcmp(argv[i], "-size_max")==0){
			max_size = atoi(argv[++i]);
			max_flag = true;
			//cout << "max_size: " << max_size <<endl;
		}
		if(strcmp(argv[i], "-size_min")==0){
			min_size = atoi(argv[++i]);
			min_flag = true;
			//cout << "min_size: " << min_size <<endl;
		}
	}
	
	char* path = argv[1];
	int len = strlen(path);
	if(path[len-1] == '/')
		path[len-1] = '\0';
	FIND(path);
	//FIND(argv[1]);
	
	return 0;
}

void FIND(const char* path){
	DIR* pdir = opendir(path);
	struct dirent *pent = NULL; 
	struct stat info;
	string child_path;
	float size;
	bool print = true;
	//cout << "recursive FIND\n";
	if(!pdir){
		cout << "fail to open the directory: " << path << endl;
		return;
	}

	while((pent = readdir(pdir))!=NULL){
		print = true;
		
		
		child_path = path;//child_path = path + "/" + pent->d_name;
		child_path += '/';
		child_path += pent->d_name;
		//child_path += '\0';
		
		if(pent->d_type & DT_DIR){ //it's a directory
		
			if(strcmp(pent->d_name,".")==0||strcmp(pent->d_name,"..")==0)
				continue;
			
			FIND(child_path.c_str());
		}
		
		if(stat(child_path.c_str(), &info)<0)//get the informantion about a file
				cout << "error on stat() !! child_path: " << child_path << endl;

		/*if(node){
			if(info.st_ino == inode_num)
				print = true;
			else
				print = false;
		}
		if(name){
			if(info.st_ino ==inode_num)
				print = true;
			else
				print = flase;
		}
		if(min_flag){
			if(size>=min_size)
				print = true;
			else
				print = false;
		}
		if(max_flag){
			if(size<=max_size)
				print = true;
			else 
				print = false;
		}*/
		if(inode && info.st_ino != inode_num)
			print = false;
			
		if(name && filename != pent->d_name)
			print = false;

		size = (float)info.st_size/(1024.0 * 1024.0); // file's size or directory's size
		if(min_flag && size < min_size)
			print = false;
		if(max_flag && size > max_size)
			print = false;

		if(print)
			cout << child_path << " " << pent->d_ino << " " << size << " MB\n";

		//cout << "READING: " << path << "/" << pent->d_name << endl;
	}

	closedir(pdir);
	return;
}