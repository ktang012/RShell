#include <queue>
#include <cstring>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pwd.h>

using namespace std;
using namespace boost;

unsigned const NORD = 0; // no rd
unsigned const INRD = 1; // input rd
unsigned const SINRD = 2; // string inrd
unsigned const OURD = 3; // output rd
unsigned const AOURD = 4; // append ourd
unsigned const FDOURD = 5; // fd ourd
unsigned const PIPE = 6;
unsigned const RDFAIL = 7; // syntax error

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
    char_separator<char> delim(" ", ";<>&|#") ;
    tokenizer<char_separator<char>> mytok(s, delim);

    for (auto i = mytok.begin(); i != mytok.end(); ++i) {
        l.push(*i);
    }
}

void pop_q(queue<string> &q) {
    while(!q.empty()) {
        q.pop();
    }
}

unsigned set_rd(queue<string> q) {
    unsigned char_count = 0;
    while(!q.empty() && q.front() == "<") {
        ++char_count;
        q.pop();
    }
    if (char_count == 1) { // only "<"
        return INRD;
    }
    else if (char_count == 3) { // only "<<<"
        return SINRD;
    }
    else if (char_count != 0) {
        return RDFAIL;
    }
    while (!q.empty() && q.front() == ">") {
        ++char_count;
        q.pop();
    }
    if (char_count == 1) { // only ">"
        return OURD;
    }
    else if (char_count == 2) { // only ">>"
        return AOURD;
    }
    else if (char_count != 0) {
        return RDFAIL;
    }
    if (!q.empty() && q.front() == "|") {
        q.pop();
        return PIPE;
    }
    return RDFAIL;
}

string get_fn(queue<string> &q) {
    string fn = q.front();
    q.pop();
    return fn;
}

bool pop_rd_success(queue<string> &q, const unsigned rd, string &fn) {
    if (rd == NORD) {
        return true;
    }
    else if (rd == INRD || rd == OURD || rd == PIPE) {
        q.pop();
        if (rd != PIPE) {
            fn = get_fn(q);
        }
        return true;
    }
    else if (rd == SINRD) {
        q.pop();
        q.pop();
        q.pop();
        fn = get_fn(q);
        return true;
    }
    else if (rd == AOURD) {
        q.pop();
        q.pop();
        fn = get_fn(q);
        return true;
    }
    return false;
}

bool is_num(const string &s) {
    auto it = s.begin();
    while (it != s.end() && isdigit(*it)) {
        ++it;
    }
    return (!s.empty() && it == s.end());
}
bool has_input_fd(vector<string> &v, const unsigned rd, unsigned &fd) {
    if ((rd == OURD || rd == AOURD) && v.size() != 0 && is_num(v.at(v.size()-1))) {
        fd = atoi(v.at(v.size()-1).c_str());
        v.pop_back();
        return true;
    }
    return false;
}

/* modifies queue accordingly, returns redirection flag, and filename to be redirected */
/* also checks for #> or #>> cases */
unsigned get_rd(queue<string> &q, vector<string> &v, string &fn, unsigned &fd) {
    unsigned rd = set_rd(q);
    has_input_fd(v,rd, fd);
    if (pop_rd_success(q, rd, fn)) {
        return rd;
    }
    return RDFAIL;
}

void fill_sub_cmd(queue<string> &q, vector<string> &v) {
    while (!q.empty() && q.front() != "<" && q.front() != ">" && q.front() != "|") {
        v.push_back(q.front());
        q.pop();
    }
}

void print_vector(const vector<string> &v) {
    if (v.size() == 0) {
        return;
    }
    for (unsigned i = 0; i < v.size(); ++i) {
        cout << "[" << v.at(i) << "] ";
    }
    cout << endl;
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

void vinfo(const vector<string> &v, const unsigned rd) {
    cout << "RD: " << rd << " and vector size: " << v.size() << endl;
    print_vector(v);
}

void qinfo(queue<string> &q) {
    cout << "Queue size: " << q.size() << endl;
    print_queue(q);
}

bool contains_pipe(queue<string> q) {
    if (!q.empty() && q.front() == "|") {
        q.pop();
        if (!q.empty() && q.front() != "&" && q.front() != "|" && q.front() != ";" && q.front() != "#") {
            return true;
        }
    }
        return false;
}

void parse_connectors(queue<string> &v, queue<string> &q) {
    while (!q.empty() && q.front() != "&" && q.front() != "|" && q.front() != ";" && q.front() != "#") {
        v.push(q.front());
        q.pop();
    }
    if (contains_pipe(q)) { /* if pipe add pipe to v and continue parsing */
        v.push(q.front());
        q.pop();
        parse_connectors(v, q);
    }
}

bool cmd_exit(queue<string> &v) {
    if (iequals(v.front(), "exit")) {
        return true;
    }
    return false;
}

void to_arr_cstring(vector<string> &s, vector<char*> &cs) {
    for (unsigned i = 0; i != s.size(); ++i) {
        cs[i] = &s[i][0];
    }
}

bool begin_exec(vector<char*> &cs, const unsigned rd, const string &fn, const unsigned input_fd) {
    int status;
    int pid = fork();
    if (pid < 0) { // error
        perror("FORK");
        _exit(-1);
    }
    else if (pid == 0) { // child
        if (rd == OURD) {
            int oldfd, newfd;
            if (-1 == (newfd = open(fn.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR))) {
                perror("OPEN OURD");
                _exit(-1);
            }
            if (-1 == (oldfd = dup(1))) {
                perror("DUP OURD");
                _exit(-1);
            }
            if (-1 == dup2(newfd, 1)) {
                perror("DUP2 OURD");
                _exit(-1);
            }
        }
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
        queue<string> cmd;
        //vector<string> cmd;
        while (!list.empty()) {
            //qinfo(list); // debugger
            bool exec_success = false;
            if (list.front() == "#") {
                break;
            }
            parse_connectors(cmd, list);
            if (cmd.empty()) {
                cout << "Syntax error: formatting issue in command" << endl;
                break;
            }
            else if (cmd_exit(cmd)) {
                return 0;
            }
            else {
                while (!cmd.empty()) {
                    //qinfo(cmd);
                    vector<string> sub_cmd;
                    string filename; unsigned input_fd;
                    /* stops at rd symbols, sets flags, and gets input_fd */
                    fill_sub_cmd(cmd, sub_cmd);
                    unsigned rd = get_rd(cmd, sub_cmd, filename, input_fd);
                    fill_sub_cmd(cmd, sub_cmd);
                    //vinfo(sub_cmd, rd);
                    /* figure out how to connect multiple rd and pipe */
                    vector<char*> cmd_cstring(sub_cmd.size() + 1);
                    to_arr_cstring(sub_cmd, cmd_cstring);
                    // cout << "FILENAME " << filename << endl;
                    exec_success = begin_exec(cmd_cstring, rd, filename, input_fd);
                }
                if (!connect_success(exec_success, list)) {
                    break;
                }
            }
            pop_q(cmd);
        }
    }
    return 0;
}
