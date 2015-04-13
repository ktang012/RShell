#include <iostream>
#include <unistd.h>
#include <vector>
#include <queue>
#include <string.h>
using namespace std;

void parse_cmd_arg(string &input, queue<string> q) {


int main(int argc, char* argv[]) {
    string cmd_input;
    queue<string> parsed_input;
    while (1) {
        // Get input
        // Parse input
            // 1) Add spaces between every connector - ?
            // 2) Use tokenizing methods to get substrings
            // 3) Add substrings onto queue
        // While queue is NOT empty
            // 1) Pop first item
                // Check if "exit"
            // 2) Continue popping flags UNTIL connector - stop
            // 3) Execute command
            // 4) Pop connector
        cin >> cmd_input;
        parse_cmd_arg(cmd_input);

        while (!parsed_input.empty()) {


    }
    return 0;
}
