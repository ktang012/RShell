#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

using namespace std;

bool isDIR (char* c) 
{
	struct stat buff;
	if (stat(c, &buff) == -1) 
	{
		perror("stat");
		exit(1);
	}
	if (buff.st_mode & S_IFDIR)	return true;
	return false;
}

void mymove(char*file1, const char* file2) 
{
	errno = 0;
	if (link(file1,file2) == -1)
	{
		perror("link");
		exit(1);
	}
	if (unlink(file1) == -1)	//rm file
	{
		perror("unlinK");
		exit(1);
	}
}

bool exists(char* file) 
{
	struct stat buff;
	if (stat(file, &buff) == -1) return false;	//file does not exist
	return true;	//file exists
}

int main(int agrc, char** argv) 
{
	string s,s1,s2;	//intialize string

	if(!exists(argv[1])) //	does file 1 exist?
	{
		perror("stat");
		return -1;
	}

	if (exists(argv[2]))	//does file 2 exist
	{
		if (isDIR(argv[2]))		//is  it a directory? 
		{
			s1 = argv[1];	//assign file name to s1
			s2 = argv[2];	//assign directory file to s2
			s  = s2 + "/" + s1;		//form file path 
			mymove(argv[1], s.c_str());	//change the file path of s1 to new filepath s
		}
		else 
		{
			cout << argv[2]	<< "is a not directory." << endl;
		}
	}
	else 
	{
		mymove(argv[1], argv[2]); //arg 2 is a filepath	
	}
	return 0;
}
