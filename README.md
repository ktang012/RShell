# RShell

RShell is a simple shell program written by CS100 students. It should take in a bash command, or
several commands, and execute the command(s) and flag(s), if any, accordingly. Basic connectors will be used to link them
together.

## Basic Usage

RShell supports only basic bash commands and connectors.

### Commands and Flags

RShell reads in commands in the format: `command` `flags` `logical connector`

RShell will always first take in a `command` followed by `flags` if applicable.

`exit` will exit the shell if executed as a command. It is NOT case sensitive.

### Logical Connectors

RShell uses the following logic connector: `&&`, `||`, and `;` to connect the execution of commands in a logical fashion.
 
`&&` will execute the next command, if and only if the previous command executes.

`||` will execute the next command, if and only if the previous command fails to execute.

`;` will always execute the next command.

Everything after `#` will be ignored and treated as a comment. Logical connectors without a preceding command will fail. Logical connectors without a following command will be ignored.

## Known Bugs and Issues

- Everything after `#` will be treated as a comment regardless of its integrity in the command.
- The use of `echo` will echo everything following it until a `&`, `|`, `;`, or `#` is reached.
- Any `"` will be included in the echo.
- Able to run multiple instances of RShell in itself.


