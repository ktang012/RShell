#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "Timer.h"

using namespace std;

bool file_exists(char *file) {
    ifstream opened(file);
    if (opened.good()) {
       return true;
    }
    return false;
}

bool is_dir(char *file) {
    struct stat info;
    if (stat(file, &info) == -1) {
        perror("STAT");
        exit(1);
    }
    if (S_ISDIR(info.st_mode)) {
        return true;
    }
    return false;
}

void fstream_cp(char *s, char *d) {
    ifstream src(s);
    ofstream dest(d);
    char c;
    while (src.get(c)) {
        dest.put(c);
    }
    src.close();
    dest.close();
}

void rwbuff_cp(char *s, char *d) {
    int src, dest;
    if ((src = open(s, O_RDONLY)) == -1) {
        perror("OPEN SRC");
        exit(1);
    }
    if ((dest = open(d, O_RDWR | O_CREAT)) == -1) {
        perror("OPEN DEST");
        exit(1);
    }

    int size;
    char buff[BUFSIZ];
    if ((size = read(src, buff, sizeof(buff))) == -1) {
        perror("READ");
        exit(1);
    }
    while (size > 0) {
        if (write(dest, buff, size) == -1) {
            perror("WRITE");
            exit(1);
        }
        if ((size = read(src, buff, sizeof(buff))) == -1) {
            perror("READ");
            exit(1);
        }
    }

    if (close(src) == -1) {
        perror("CLOSE SRC");
        exit(1);
    }
    if (close(dest) == -1) {
        perror("CLOSE DEST");
        exit(1);
    }
}

void rw_cp(char *s, char *d) {
    int src, dest;
    if ((src = open(s, O_RDONLY)) == -1) {
        perror("OPEN SRC");
        exit(1);
    }
    if ((dest = open(d, O_RDWR | O_CREAT)) == -1) {
        perror("OPEN DEST");
        exit(1);
    }

    int size;
    char buff[BUFSIZ];
    if ((size = read(src, buff, sizeof(buff))) == -1) {
        perror("READ");
        exit(1);
    }
    while (size > 0) {
        if (write(dest, buff, size) == -1) {
            perror("WRITE");
            exit(1);
        }
        if ((size = read(src, buff, 1)) == -1) {
            perror("READ");
            exit(1);
        }
    }

    if (close(src) == -1) {
        perror("CLOSE SRC");
        exit(1);
    }
    if (close(dest) == -1) {
        perror("CLOSE DEST");
        exit(1);
    }
}

void pTime(Timer &t, const string fname) {
    double w, u, s;
    t.elapsedTime(w, u, s);
    cout << "doing function: " << fname << endl;
    cout << "user: " << u << endl;
    cout << "system: " << s << endl;
    cout << "wallclock: " << w << endl;
    cout << endl;
    t.start();
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        cout << "include source file, destination file";
        cout << " and an optional parameter" << endl;
        return 1;
    }

    // exec file1 -> file2 parameter

    char *src = argv[1];
    char *dest = argv[2];
    unsigned option = 0;
    if (argc == 4) {
        option = 1;
    }

    if (is_dir(src)) {
        cout << "error: src is a directory" << endl;
        return 1;
    }
    if (!file_exists(src)) {
        cout << "file does NOT exists" << endl;
        exit(1);
    }
    if (file_exists(dest)) {
        cout << "file exits" << endl;
        exit(1);
    }

    if (option == 1) {
        Timer t;
        t.start();

        fstream_cp(src, dest);
        pTime(t, "fstream");

        rw_cp(src, dest);
        pTime(t, "rw_cp");

        rwbuff_cp(src, dest);
        pTime(t, "rwbuff_cp");
    }
    else {
        Timer t;
        t.start();

        rwbuff_cp(src, dest);
        pTime(t, "rwbuff_cp");
    }


    return 0;
}
