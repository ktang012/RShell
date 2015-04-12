#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    string cmd_input;
    while (1) {
        cout << "$ ";
        cin >> cmd_input;
        // Parse
            // check for errors while parsing
                // [always exec] [maybe flag] [maybe connector]
                // parse input
                    // 1) first is exec
                    // 2) maybe everything after is a flag
                    // 3) until connector
            // add spaces in between connectors
            // parse using strtok
            //




        // break out of loop if "exit"
    }
    return 0;
}
