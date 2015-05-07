#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h> //exit 1
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
using namespace std;

void remove_dir(const char * dir)
{
	int ret = rmdir(dir);
	if(ret == -1){
		cerr << __LINE__ <<  ": ";
		cerr << dir;
		perror("There was an error in rmdir()");
		exit(1);
	}
}

void remove_file(const char * file)
{

	int ret = unlink(file);
	if(ret == -1){
		cerr << __LINE__ <<  ": ";
		cerr << file;
		perror("There was an error in unlink()");
		exit(1);
	}
}

bool isDir(const char * path)
{

	struct stat st;
	if(stat(path, &st) < 0){
		cerr << __LINE__ <<  ": ";
		cerr << path;
		perror(" stat");
		exit(1);
	}
	else {
		if(S_ISDIR(st.st_mode)){
			return true;
		}
	}
	return false;
}

void recursive(const char * path)
{
	DIR * dirp;
	dirp = opendir(path);
	if(dirp == NULL){
		cerr << __LINE__ <<  ": ";
		cerr << path;
		perror("opendir");
		exit(1);
	}

	struct dirent *fl;
	errno = 0;

	while((fl = readdir(dirp)) != NULL) {
		string tmp = path;
		tmp += "/" + string(fl->d_name);
		struct stat st;
		if(lstat(tmp.c_str(), &st) < 0){
			cerr << __LINE__ <<  ": ";
			perror("stat");
			exit(1);
		}
		else {
			if(S_ISDIR(st.st_mode)){
				string sfl = string(fl->d_name);
				if(sfl[0] != '.'){
					recursive(tmp.c_str());
					remove_dir(tmp.c_str());
				}
			} else {
				remove_file(tmp.c_str());
			}
		}
	}

	if(errno != 0){
		cerr << __LINE__ <<  ": ";
		perror("readdir");
		exit(1);
	} 
	if(closedir(dirp) == -1){
		cerr << __LINE__ <<  ": ";
		perror("closedir");
		exit(1);
	}
}


int main(int argc, char** argv)
{
	if(argc < 2){
		cerr << "At least 1 file needs to be passed in." <<endl;
		exit(1);
	} 
	else if(strcmp(argv[1], "-r") == 0){
		for(int i = 2; i < argc; ++i)
		{
			string tmp = "./" + string(argv[i]);
			
			if(isDir(tmp.c_str())){
				recursive(tmp.c_str());
				remove_dir(tmp.c_str());
			}
			else 
				remove_file(tmp.c_str());
		}
	} else {
		for(int i = 1; i < argc; ++i){
            string tmp = "./" + string(argv[i]);
			remove_file(tmp.c_str());
		}
	}

	return 0;

}
	
