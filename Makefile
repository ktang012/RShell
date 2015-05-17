all: rshell

rshell:
	mkdir -p ./bin
	g++ -Wall -Werror -std=c++11 -pedantic ./src/rshell.cpp -o ./bin/rshell
	g++ -Wall -Werror -std=c++11 -pedantic ./src/cp.cpp -o ./bin/cp
	g++ -Wall -Werror -std=c++11 -pedantic ./src/ls.cpp -o ./bin/ls
	g++ -Wall -Werror -std=c++11 -pedantic ./src/mv.cpp -o ./bin/mv
	g++ -Wall -Werror -std=c++11 -pedantic ./src/rm.cpp -o ./bin/rm

