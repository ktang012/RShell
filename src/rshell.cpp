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

/* will need to implement file tracing for redirection and piping */
/* idea 1:
 * have vector of rd symbols and corresponding arguments, possibly rd flags too..
 * apply exec to each accordingly
 * parse cmd to fill rd symbol and argument vectors when appropriate
 * ex. ls > file1 > file2 - ls fills file1 but then file1 gives content to file2
 * ex. ls > file1 && > file2 - ls fills file1, finishes cmd. fills file2 with nothing
 */

unsigned const NORD = 0; // no rd
unsigned const INRD = 1; // input rd
unsigned const SINRD = 2; // string inrd
unsigned const OURD = 3; // output rd
unsigned const AOURD = 4; // append ourd
unsigned const FDOURD = 5; // fd ourd
unsigned const FDAOURD = 6; // fd aourd
unsigned const PIPE = 7;
unsigned const NO_PIPE = 8;
unsigned const RDFAIL = 9; // syntax error
string const NO_FILE = "NO FILE";

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
    return NORD;
}

bool get_fn(queue<string> &q, string &fn) {
    if (q.empty()) {
        return false;
    }
    fn = q.front();
    q.pop();
    return true;
}

bool pop_rd_success(queue<string> &q, const unsigned rd, string &fn) {
    if (rd == NORD) {
        return true;
    }
    else if (rd == INRD || rd == OURD || rd == PIPE) {
        q.pop();
        if (rd != PIPE) {
            return (get_fn(q, fn));
        }
        return true;
    }
    else if (rd == SINRD) {
        q.pop();
        q.pop();
        q.pop();
        return (get_fn(q, fn));
    }
    else if (rd == AOURD) {
        q.pop();
        q.pop();
        return (get_fn(q, fn));
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

/* modifies queue accordingly, returns redirection flag, and filename to be redirected */
/* also checks for #> or #>> cases */

void fill_sub_cmd(queue<string> &q, vector<string> &v) {
    while (!q.empty() && q.front() != "<" && q.front() != ">" && q.front() != "|") {
        v.push_back(q.front());
        q.pop();
    }
}

void print_vector(const vector<string> &v) {
    if (v.empty()) {
        cout << "[NULL] ";
        return;
    }
    for (unsigned i = 0; i < v.size(); ++i) {
        cout << "[" << v.at(i) << "] ";
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

bool has_input_fd(vector<string> &v, const unsigned rd, unsigned &fd) {
    if ((rd == OURD || rd == AOURD) && v.size() != 0 && is_num(v.at(v.size()-1))) {
        fd = atoi(v.at(v.size()-1).c_str());
        v.pop_back();
        return true;
    }
    return false;
}


int parse_pipes(queue<string> &q, vector< vector<string> > &v, queue<pair<unsigned, string> > &p, queue<unsigned> &pipes) {
    int pipe_count = 0;
    for (unsigned i = 0; !q.empty(); ++i) {
        vector<string> sub_v;
        while (!q.empty() && q.front() != "<" && q.front() != ">" && q.front() != "|") {
            sub_v.push_back(q.front());
            q.pop();
        }
        unsigned rd = set_rd(q);
        string fn; unsigned fd;
        // bool fdstate = has_input_fd(sub_v, rd, fd); // implement fdourd and fdaourd here
        if (pop_rd_success(q, rd, fn)) {
            if (rd == NORD || rd == PIPE) {
                fn = NO_FILE;
            }
            p.push(make_pair(rd, fn));
        }
        else {
            return -1;
        }
        if (rd == PIPE) {
            pipes.push(PIPE);
            ++pipe_count;
        }
        else {
            pipes.push(NO_PIPE);
        }
        v.push_back(sub_v);
    }
    return pipe_count;
}

void print_pair(const pair<unsigned, string> &p) {
    cout << "(" << p.first << ", " << p.second << ") ";
}

void print_qpair(queue< pair<unsigned, string> > p) {
    if (p.empty()) return;
    while (!p.empty()) {
        print_pair(p.front());
        p.pop();
    }
    cout << endl;
}

void print_2dvector(const vector< vector<string> > &v) {
    if (v.empty()) return;
    for (unsigned i = 0; i < v.size(); ++i) {
        cout << "(" << i << ") ";
        print_vector(v.at(i));
        cout << endl;
    }
}

void convert_vcmd_to_cscmd(vector< vector<string> > &v, vector< vector<char*> > &cs) {
    for (unsigned i = 0; i < v.size(); ++i) {
        vector<char*> sub_cs(v.at(i).size()+1);
        to_arr_cstring(v.at(i), sub_cs);
        cs.push_back(sub_cs);
    }
}

void print_cmd_pair (vector< vector<string> > &v, queue< pair<unsigned, string> > p, queue<unsigned> pipes) {
    if (v.size() != p.size() && v.size() != pipes.size()) {
        return;
    }
    for (unsigned i = 0; i < v.size(); ++i) {
        print_vector(v.at(i));
        print_pair(p.front());
        cout << "[" << pipes.front() << "] ";
        pipes.pop();
        p.pop();
        cout << endl;
    }
}

bool check_size(vector< vector<string> > &v, queue< pair<unsigned, string> > p, queue<unsigned> pipes) {
    return v.size() == p.size() && v.size() == pipes.size() && p.size() == pipes.size();
}

bool make_pipes(int fd_2d[][2], const unsigned num_pipes) {
    for (unsigned i = 0; i < num_pipes; ++i) {
        if (-1 == pipe(fd_2d[i])) {
            perror("PIPE");
            _exit(-1);
        }
    }
    return true;
}

void p_2darr(int arr[][2], const unsigned num_pipes) {
    for (unsigned i = 0; i < num_pipes; ++i) {
        cout << i << ": ";
        for (unsigned j = 0; j < 2; ++j) {
            cout << arr[i][j] << " ";
        }
        cout << endl;
    }
}

vector<char*> construct_subv (vector< vector<char*> > &v, unsigned &i) {
    vector<char*> subv;
    for (unsigned j = 0; j < v.at(i).size(); ++j) {
        subv.push_back(v.at(i).at(j));
    }
    ++i; /* to keep it concurrent with queues */
    return subv;
}

pair<unsigned, string> construct_subq (queue< pair<unsigned, string> > &q) {
    pair<unsigned, string> subq;
    subq = make_pair(q.front().first, q.front().second);
    q.pop();
    return subq;
}

unsigned construct_pipe (queue<unsigned> &pipes) {
    unsigned pipe = pipes.front();
    pipes.pop();
    return pipe;
}

int has_multi_rd(vector< vector<char*> > &v, unsigned i) {
    vector<char*> sub_v;
    while (sub_v.size() <= 1 && i < v.size()) {
        sub_v.clear();
        sub_v = construct_subv(v, i);
    }
    if (sub_v.size() > 1) {
        return i -1;
    }
    return i;
}

void equalize(queue< pair<unsigned, string> > &q, queue<unsigned> &p, int i) {
    for (unsigned j = 0; j < i-1 && !q.empty() && !p.empty(); ++j) {
        q.pop();
        p.pop();
    }
    /* not poping p so we can construct a new p */
    q.pop();
}

void pdata(const vector<char*> &v, const pair<unsigned, string> &p, const unsigned pipe) {
    for (unsigned i = 0; i < v.size() -1; ++i) {
        cout << "[" << v.at(i) << "] ";
    }
    cout << "(" << p.first << ", " << p.second << ") ";
    if (pipe == PIPE) {
        cout << "PIPE" << endl;
    }
    else {
        cout << "NO PIPE" << endl;
    }
}

bool begin_exec(vector< vector<char*> > &v, queue< pair<unsigned, string> > &q, queue<unsigned> &pipes, unsigned num_pipes) {
    int status;
    int fd_2d [num_pipes][2];
    make_pipes(fd_2d, num_pipes);
    for (unsigned i = 0; i < v.size() && !q.empty() && !pipes.empty();) { // construct_subv handles i
        vector<char*> sub_v = construct_subv(v, i);
        pair<unsigned, string> sub_q = construct_subq(q);
        unsigned rd = sub_q.first;
        string fn = sub_q.second;
        unsigned pipe = construct_pipe(pipes);
        int merge_count = has_multi_rd(v, i) - i;
        if (merge_count > 0) {
            // cout << "Preparing to merge next: " << merge_count << " packets!" << endl;
            equalize(q, pipes, merge_count);
            i += merge_count;
            pipe = construct_pipe(pipes);
        }
        pdata(sub_v, sub_q, pipe);
        int pid = fork();
        if (pid < 0) {
            perror("FORK");
            _exit(-1);
        }
        else if (pid == 0) { /* child */
            if (pipe == PIPE) {
                /* do pipe shit */

            }
            else {


            }

        }




    }
    return true;
}


int main(){
    while (1) {
        string cmd_input;
        queue<string> list;
        pcmd_prompt();
        getline(cin, cmd_input);
        parse_into_queue(list, cmd_input);
        queue<string> cmd;
        while (!list.empty()) {
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
                qinfo(cmd);
                vector< vector<string> > v_cmd;
                queue< pair<unsigned, string> > rd_file;
                queue<unsigned> pipes;
                int num_pipes = parse_pipes(cmd, v_cmd, rd_file, pipes);
                print_cmd_pair(v_cmd, rd_file, pipes);
                cout << "PIPES FOUND: " << num_pipes << endl;
                if (check_size(v_cmd, rd_file, pipes) && v_cmd.size() != 0 && num_pipes != -1) {
                    vector< vector<char*> > cs_cmd;
                    convert_vcmd_to_cscmd(v_cmd, cs_cmd);
                    exec_success= begin_exec(cs_cmd, rd_file, pipes, num_pipes);
                }
                else {
                    cout << "Syntax error: formatting issue with redirection or piping" << endl;
                    break;
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
