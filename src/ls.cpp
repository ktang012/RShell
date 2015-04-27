#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <queue>
#include <vector>
#include <algorithm>

using namespace std;

void filter_argv(int argc, char* argv[], queue<string> &p, vector<string> &f) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            f.push_back(argv[i]);
        }
        else {
            p.push(argv[i]);
        }
    }
}

void p_vector (const vector<string> &v) {
    for (unsigned i = 0; i < v.size(); ++i) {
        cout << v.at(i) << " ";
    }
    cout << endl;
}

void sort_vector(vector<string> &v) {
    sort(v.begin(), v.end());
}

unsigned get_flags(const vector<string> &v) {
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

bool flag_ALL(unsigned flags) {
    if ((flags & 1) == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool flag_LONG(unsigned flags) {
    if ((flags & 2) == 2) {
        return true;
    }
    else {
        return false;
    }
}

bool flag_RECURSIVE(unsigned flags) {
    if ((flags & 4) == 4) {
        return true;
    }
    else {
        return false;
    }
}

void push_to_files (DIR *dirp, vector<string> &files) {
    struct dirent *filespecs;
    errno = 0;
    while ((filespecs = readdir(dirp)) != NULL) {
        if (filespecs->d_name[0] != '.') {
            files.push_back(filespecs->d_name);
        }
    }
    if (errno != 0) {
        perror("READDIR");
        exit(1);
    }
    if (closedir(dirp) == -1) {
        perror("CLOSEDIR");
        exit(1);
    }
}

void push_all_to_files (DIR *dirp, vector<string> &files) {
    struct dirent *filespecs;
    errno = 0;
    while ((filespecs = readdir(dirp)) != NULL) {
        files.push_back(filespecs->d_name);
    }
    if (errno != 0) {
        perror("READDIR");
        exit(1);
    }
    if (closedir(dirp) == -1) {
        perror("CLOSEDIR");
        exit(1);
    }
}

void exec_long(DIR *dirp, vector<string> &files) {



}

void apply_flags(unsigned flags, queue<string> &paths, vector<string> &files) {
    while (!paths.empty()) {
        DIR *dirp;
        if ((dirp = opendir((paths.front()).c_str())) == NULL) {
            perror("OPENDIR");
            exit(1);
        }
        paths.pop();

        if (flag_ALL(flags)) {
           push_all_to_files(dirp, files);
        }
        else {
           push_to_files(dirp, files);

        }
        sort_vector(files);

        if (flag_LONG(flags)) {
            exec_long(dirp, files);
        }
        else {
            p_vector(files);
        }

        if (flag_RECURSIVE(flags)) {
            /* look for directories */
            /* apply_flags(flags, new paths, new files) */
        }
    }
}

void init_ls(unsigned flags, queue<string> &paths) {
    vector<string> files;
    if (paths.empty()) {
        paths.push(".");
    }
    apply_flags(flags, paths, files);
}

int main(int argc, char* argv[]) {
    if (argc == 0) {
        cout << "no args" << endl;
    }
    else {
        vector<string> flags;
        queue<string> paths;
        filter_argv(argc, argv, paths, flags);
        unsigned set_flags = get_flags(flags);
        init_ls(set_flags, paths);
    }
    return 0;
}
