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
#include <pwd.h>
#include <grp.h>

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
        cout << v.at(i) << "  ";
    }
    cout << endl;
}

bool case_insensitive(const string &a, const string &b) {
    for (unsigned i = 0; i < a.length() && i < b.length(); ++i) {
        if (tolower(a.at(i)) < tolower(b.at(i))) {
            return true;
        }
        else if (tolower(a.at(i)) > tolower(b.at(i))) {
            return false;
        }
    }
    return (a.length() < b.length());
}


void sort_vector(vector<string> &v) {
   stable_sort(v.begin(), v.end(), case_insensitive);
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
            // cout << "PUSHING: " << filespecs->d_name << endl;
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
        // cout << "PUSHING: " << filespecs->d_name << endl;
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

void getperms(mode_t st_mode, string &perms) {
    if (S_ISDIR(st_mode))
        perms.at(0) = 'd';
    else if (S_ISCHR(st_mode))
        perms.at(0) = 'c';
    else if (S_ISBLK(st_mode))
        perms.at(0) = 'b';
    else if (S_ISFIFO(st_mode))
        perms.at(0) = 'p';
    else if (S_ISLNK(st_mode))
        perms.at(0) = 'l';
    else if (S_ISSOCK(st_mode))
        perms.at(0) = 's';

    if (S_IRUSR & st_mode)
        perms.at(1) = 'r';
    if (S_IWUSR & st_mode)
        perms.at(2) = 'w';
    if (S_IXUSR & st_mode)
        perms.at(3) = 'x';

    if (S_IRGRP & st_mode)
        perms.at(4) = 'r';
    if (S_IWGRP & st_mode)
        perms.at(5) = 'w';
    if (S_IXGRP & st_mode)
        perms.at(6) = 'x';

    if (S_IROTH & st_mode)
        perms.at(7) = 'r';
    if (S_IWOTH & st_mode)
        perms.at(8) = 'w';
    if (S_IXOTH & st_mode)
        perms.at(9) = 'x';
}

void gettime(struct stat &t, string &s) {
    // deletes extra info such as weekday, seconds, and years
    // time format: Mmm dd hh:mm
    s = ctime(&t.st_mtime);
    s.erase(0, 4);
    s.erase(12,8);
    s.at(s.size()-1) = '\0';
}

void get_usr_grp (const struct stat &info, string &u, string &g) {
    struct passwd usr = *getpwuid(info.st_uid);
    struct group grp = *getgrgid(info.st_gid);
    if (&usr == NULL) {
        perror("GETPWUID");
        exit(1);
    }
    if (&grp == NULL) {
        perror("GETGRGID");
        exit(1);
    }
    u = usr.pw_name;
    g = grp.gr_name;
}

void exec_long (vector<string> &files, string &curr_directory) {
    struct stat info;
    for (unsigned i = 0; i < files.size(); ++i) {
        //cout << "checking: " << curr_directory << "/" << files.at(i) << endl;
        if (stat((curr_directory + "/"+ (files.at(i))).c_str(), &info) == -1) {
            perror("STAT");
            exit(1);
        }
        string perms = "----------";
        getperms(info.st_mode, perms);
        string usr, grp;
        get_usr_grp(info, usr, grp);
        string time;
        gettime(info, time);
        // perms - # of folders - user - group - size in bytes - date/time last modified
        cout << perms << " " << info.st_nlink << " " << usr << " " << grp << " ";
        cout << info.st_size << " " << time << " " << files.at(i) << endl;
    }
}

void apply_flags(unsigned flags, queue<string> &paths) {
    while (!paths.empty()) {
        DIR *dirp;
        vector<string> files;
        if ((dirp = opendir((paths.front()).c_str())) == NULL) {
            perror("OPENDIR");
            exit(1);
        }

        string curr_directory = paths.front();
        // cout << "GOING TO DIRECTORY: " << paths.front() << endl;
        paths.pop();

        if (flag_ALL(flags)) {
           push_all_to_files(dirp, files);
        }
        else {
           push_to_files(dirp, files);

        }
        sort_vector(files);

        if (flag_LONG(flags)) {
            exec_long(files, curr_directory);
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
    if (paths.empty()) {
        paths.push(".");
    }
    apply_flags(flags, paths);
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
