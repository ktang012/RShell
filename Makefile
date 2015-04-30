all: rshell

rshell:
	mkdir -p ./bin
	g++ -Wall -Werror -std=c++11 -pedantic ./src/rshell.cpp -o ./bin/rshell
	g++ -Wall -Werror -std=c++11 -pedantic ./src/cp.cpp -o ./bin/cp
