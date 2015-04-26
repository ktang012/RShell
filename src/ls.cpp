#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <vector>

using namespace std;

void filter_argv(int argc, char* argv[], const vector<string> &p, const vector<string> &f) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            f.push_back(argv[i]);
        }
        else {
            p.push_back(argv[i]);
        }
    }
}

void p_vector (const vector<string> &v) {
    for (unsigned i = 0; i < v.size(); ++i) {
        cout << v.at(i) << " ";
    }
    cout << endl;
}

unsigned check_flags(const vector<string> &v) {
    int bv = 0; // bit vector for flags
    for (unsigned i = 0; i < v.size(); ++i) {
        for (unsigned j = 1; j < v.at(i).size(); ++j) {
            if (v.at(i).at(j) == 'a') {
                bv = bv | 1;
            }
            else if (v.at(i).at(j) == 'l') {
                bv = bv | 2;
            }
            else if (v.at(i).at(j) == 'R') {
                bv = bv | 4;
            }
            else {
                cout << "Invalid flag" << endl;
                exit(1);
            }
        }
    }
    return bv;
}

void begin_ls(unsigned flags, vector<string> &paths) {









}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "no args" << endl;
    }
    else {
        vector<string> paths, flags;
        filter_argv(argc, argv, paths, flags);
        unsigned set_flags = check_flags(flags);
        begin_ls(set_flags, paths);


    }
    return 0;
}
