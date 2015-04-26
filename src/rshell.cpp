#include <queue>
#include <cstring>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

using namespace std;
using namespace boost;

void pcmd_prompt() {
    struct passwd *user;
    user = getpwuid(getuid());
    if (user->pw_name == NULL) {
        perror("GETPWUID");
    }
    char host_arr[1024];
    host_arr[1023] = '\0';
    if (gethostname(host_arr, 1023) == -1) {
        perror("GETHOSTNAME");
    }
    cout << user->pw_name << "@" << host_arr << "$ ";
}

void parse_into_queue(queue<string> &l, const string &s) {
    char_separator<char> delim(" ", ";&|#") ;
    tokenizer<char_separator<char>> mytok(s, delim);

    for (auto i = mytok.begin(); i != mytok.end(); ++i) {
        l.push(*i);
    }
}

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

void qinfo(queue<string> &q) {
    cout << "Size: " << q.size() << endl;
    print_queue(q);
}

void parse_connectors(vector<string> &v, queue<string> &q) {
    while (!q.empty() && q.front() != "&" && q.front() != "|" && q.front() != ";" && q.front() != "#")
    {
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
    for (unsigned i = 0; i != s.size(); ++i) {
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

bool connect_success(bool status, queue<string> &q) {
    if (!q.empty() && q.front() == "&") {
        q.pop();
        if (!q.empty() && q.front() == "&") {
            q.pop();
            if (!status) {
                return false;
            }
            else {
                return true;
            }
        }
        else if (q.empty()) {
            cout << "Syntax error: incorrect usage of \"&&\"" << endl;
            return false;
        }
        else {
            cout << "Syntax error: incorrect usage of \"&&\"" << endl;
            return false;
        }
    }
    else if (!q.empty() && q.front() == "|") {
        q.pop();
        if (!q.empty() && q.front() == "|") {
            q.pop();
            if (status) {
                return false;
            }
            else {
                return true;
            }
        }
        else if (q.empty()) {
            cout << "Syntax error: incorrect usage of \"||\"" << endl;
            return false;
        }
        else {
            cout << "Syntax error: incorrect usage of \"||\"" << endl;
            return false;
        }
    }
    else if (!q.empty() && q.front() == ";") {
        q.pop();
        return true;
    }
    else if (!q.empty() && q.front() == "#") {
        return false;
    }
    else if (!q.empty())  {
        cout << "Syntax error: " << q.front() <<  " is not a valid connector!" << endl;
        return false;
    }
    else {
        return false;
    }
}

int main(){
    while (1) {
        string cmd_input;
        queue<string> list;
        pcmd_prompt();
        getline(cin, cmd_input);
        parse_into_queue(list, cmd_input);
        vector<string> cmd;
        while (!list.empty()) {
            qinfo(list); // debugger
            bool exec_success = false;
            if (list.front() == "#") {
                break;
            }
            parse_connectors(cmd, list);
            if (cmd.size() == 0) {
                cout << "Syntax error: formatting issue in command" << endl;
                break;
            }
            else if (cmd_exit(cmd)) {
                return 0;
            }
            else {
                vector<char*> cmd_cstring(cmd.size() + 1);
                to_arr_cstring(cmd, cmd_cstring);
                exec_success = begin_exec(cmd_cstring);
            }
            if (!connect_success(exec_success, list)) {
                break;
            }
            cmd.clear();
        }
    }
    return 0;
}
