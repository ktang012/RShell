#include <queue>
#include <cstring>
#include <iostream>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <sys/utsname.h>

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

int main(int argc, char* argv[]) {
  while (1) {
    // needs to be in while loop -- loop until "exit"
    string cmd_input;
    queue<string> list;
    pcmd_prompt();
    getline(cin, cmd_input);

    parse_into_queue(list, cmd_input);

    print_queue(list);

    cout << "init size: " << list.size() << endl;

    vector<string> cmd;
    while (!list.empty()) {
       while (!list.empty() && list.front() != "&" && list.front() != "|" && list.front() != ";") {
         cmd.push_back(list.front());
         list.pop();
       }

       if (cmd.size() == 0) {
           cout << "syntax error!" << endl;
           return 1;
       }
       else if (cmd.at(0) == "exit") {
         cout << "Exiting RShell" << endl;
         return 0;
       }
       else {
         // begin exec
         // return error flag
         // check logic to see if we continue
         // NOTE: cmd.at(0) = executable, everything else is flag
         if (cmd.size() == 1) {
             cout << "performing executable [" <<  cmd.at(0) << "] with NO flags";
         }
         else {
            cout << "performing executable [" <<  cmd.at(0) << "] with flags: ";
            for (int i = 1; i < cmd.size(); ++i) {
            cout << cmd.at(i) << " ";
            }
         }
         cout << endl;
       }

       cout << "queue size after processing cmd: " << list.size() << endl;
       print_queue(list);

       if (!list.empty() && list.front() == "&") {
         list.pop();
         if (list.front() == "&") {
             // check logic
             list.pop();
         }
         else {
             cout << "&" << list.front() << " is not a connector!" << endl;
             return 1;
         }
         cout << "queue size after processing connector: " << list.size() << endl;
         print_queue(list);
       }
       else if (!list.empty() && list.front() == "|") {
         list.pop();
         if (list.front() == "|") {
             // check logic
             list.pop();
          }
          else {
             cout << "|" << list.front() << " is not a connector!" << endl;
             return 1;
          }
          cout << "queue size after processing connector: " << list.size() << endl;
          print_queue(list);
       }
       else if (!list.empty() && list.front() == ";") {
         // go onto next
         list.pop();
         cout << "queue size after processing connector: " << list.size() << endl;
         print_queue(list);
       }
       else if (!list.empty())  {
         cout << list.front() <<  " is not a connector!" << endl;
         return 1;
       }
       else {
         cout << "no more connectors to process" << endl;
       }
       cmd.clear();
    }
  }

    return 0;
}
