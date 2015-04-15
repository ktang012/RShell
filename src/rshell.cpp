#include <queue>
#include <cstring>
#include <iostream>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;
using namespace boost;

// Cases to test:
// cd Desktop&&ls -a
// cd Desktop&[ENTER] ls -a
// cd Desktop;ls -a
// cd Desktop;ls
// ls;ls;ls;ls
// cd Desktop & ls -a
// cd Desktop;
// ls;
// cd Desktop||ls -a;vim touch.cpp&&[ENTER] ls -a
// cd Desktop
// ; clear
// && ls
// || ls -a
// NOTE: cd will NOT be tested or used!
// Re-read specs if unsure!!

// prints user and machine info
void pcmd_prompt() {
    // <sys/utsname.h> defines the struct utsname which includes info such as:
    // sysname, nodename, release, version, and machine
    // uname returns the struct with the wanted information

    string login(getlogin());
    struct utsname hostname;
    uname(&hostname);
    string nodeName(hostname.nodename);
    cout << login << "@" << nodeName << "$ ";
}

// parses input into queue for execution
void parse_into_queue(queue<string> &l, const string &s) {
    char_separator<char> delim(" ", ";&|#") ;
    tokenizer<char_separator<char>> mytok(s, delim);

    for (auto i = mytok.begin(); i != mytok.end(); ++i) {
        l.push(*i);
    }
}

// for testing purposes
void print_queue (queue<string> q) {
    if (q.empty()) {
        return;
    }
    while (!q.empty()) {
        cout << "[" << q.front() << "] ";
        q.pop();
    }
    cout << endl;
}

// reinitializes v with q until connector
void parse_connectors(vector<string> &v, queue<string> &q) {
    while (!q.empty() && q.front() != "&" && q.front() != "|" && q.front() != ";") {
        v.push_back(q.front());
        q.pop();
    }
}

bool cmd_exit(vector<string> &v) {
    if (iequals(v.at(0), "exit")) {
        return true;
    }
    return false;
}

void to_arr_cstring(vector<string> &s, vector<char*> &cs) {
    for (auto i = 0; i != s.size(); ++i) {
        cs[i] = &s[i][0];
    }
}

bool begin_exec(vector<char*> &cs) {
    int status;
    int pid = fork();
    if (pid < 0) { // error
        perror("FORK");
        _exit(-1);
    }
    else if (pid == 0) { // child
        cout << "child process" << endl;
        if (-1 == execvp(cs[0], cs.data())) {
            perror("EXEC");
            _exit(-1);
        }
        _exit(0);
    }
    else if (pid > 0) { // parent
        if (-1 == wait(&status)) {
            perror("WAIT");
        }
        if (status != 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    while (1) {
        string cmd_input;
        queue<string> list;
        pcmd_prompt();
        getline(cin, cmd_input);

        parse_into_queue(list, cmd_input);

        print_queue(list);
        cout << "init size: " << list.size() << endl;

        vector<string> cmd;
        bool exec_success = false;
        while (!list.empty()) {
            parse_connectors(cmd, list);

            if (cmd.size() == 0) {
                cout << "ERROR!" << endl;
                break;
            }
            else if (cmd_exit(cmd)) {
                cout << "Exiting RShell" << endl;
                return 0;
            }
            else {
                vector<char*> cmd_cstring(cmd.size() + 1);
                to_arr_cstring(cmd, cmd_cstring);
                exec_success = begin_exec(cmd_cstring);
            }

            cout << "queue size after processing cmd: " << list.size() << endl;
            print_queue(list);

            if (!list.empty() && list.front() == "&") {
                list.pop();
                if (!list.empty() && list.front() == "&") {
                    if (!exec_success) {
                        cout << "prev cmd fail - ending" << endl;
                        break;
                    }
                    list.pop();
                }
                else if (list.empty()) {
                    cout << "error with connector \"&\"" << endl;
                    break;
                }
                else {
                    cout << "&" << list.front() << " is not a connector!" << endl;
                    break;
                }
                cout << "queue size after processing connector: " << list.size() << endl;
                print_queue(list);
            }
            else if (!list.empty() && list.front() == "|") {
                list.pop();
                if (!list.empty() && list.front() == "|") {
                    if (exec_success) {
                        cout << "prev cmd success - ending" << endl;
                        break;
                    }
                    list.pop();
                }
                else if (list.empty()) {
                    cout << "error with connector \"|\"" << endl;
                    break;
                }
                else {
                    cout << "|" << list.front() << " is not a connector!" << endl;
                    break;
                }
                cout << "queue size after processing connector: " << list.size() << endl;
                print_queue(list);
            }
            else if (!list.empty() && list.front() == ";") {
                list.pop();
                cout << "queue size after processing connector: " << list.size() << endl;
                print_queue(list);
            }
            else if (!list.empty())  {
                cout << list.front() <<  " is not a connector!" << endl;
                break;
            }
            else {
                cout << "DONE!" << endl;
            }
            cmd.clear();
        }
    }
    return 0;
}
