#include <queue>
#include <cstring>
#include <iostream>
#include <vector>
#include <string.h>
#include <boost/tokenizer.hpp>

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

void pcmd_prompt() {
    // REMINDER: add user to prompt message
    cout << "$ ";
}

void parse_into_queue(queue<string> &l, const string &s) {
    char_separator<char> delim(" ", ";&|#") ;
    tokenizer<char_separator<char>> mytok(s, delim);

    for (auto i = mytok.begin(); i != mytok.end(); ++i) {
        l.push(*i);
    }
}

void init_cmd_queues(queue<string> &arg, queue<string> &con, queue<string> &l) {
     while (!l.empty()) {
         while (l.front() == "&" || l.front() == "|" || l.front() == ";") {
              con.push(l.front());
              l.pop();
         }
         arg.push(l.front());
         l.pop();
    }
}

void print_queue (queue<string> q) {
    while (!q.empty()) {
        cout << q.front() << " ";
        q.pop();
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    // needs to be in while loop -- loop until "exit"
    string cmd_input;
    queue<string> list;
    pcmd_prompt();
    getline(cin, cmd_input);

    parse_into_queue(list, cmd_input);

    print_queue(list);

    // while list is NOT empty
        // get executable -- check if "exit"
            // if "exit" -- exit the ENTIRE program!!
            // else place into exec cstring
        // if valid connector -- "&&", "||", ";"
            // begin execution of exec and flags
            // return any errors
            // check logic to see if we continue through list -- break if needed
        // else if flag
            // place into flags cstrings
        // continue through list

    return 0;
}
