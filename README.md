# RShell

RShell is a simple shell program written by CS100 students. It should take in a bash command, or
several commands, and execute the command(s) and flag(s), if any, accordingly. Basic connectors will be used to link them
together.

## Basic Usage

RShell supports only basic bash commands and connectors.

### Commands and Flags

RShell executes in the form of: `command flags` and the optional `logical connector` following said command. 

### Logical Operators

RShell uses the following logic operators: `&&`, `||`, and `;` to connect the execution of commands.

`&&` will execute the command following it if and only if the previous command executed successfully.

`||` will execute the command following it if and only if the previous command fails to execute properly.

`;` will execute the command following it regardless of the state of the previous command.

Everything after `#` will be ignored even if it is of a word.

## Bugs and Issues

To be updated.


