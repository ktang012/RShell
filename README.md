# RShell

RShell is a simple shell program written by CS100 students. It should take in bash commands
and execute them accordingly. Basic connectors will be used to link commands together.

## Basic Usage

RShell supports only basic bash commands and connectors.

### Commands and Flags

RShell reads in commands in the format: `command` `flags` `logical connector`

RShell will always first take in a `command` followed by `flags` if applicable.

`exit` will exit the shell if executed as a command. It is NOT case sensitive.

### Logical Connectors

RShell uses the following logic connector: `&&`, `||`, and `;` to connect the execution of commands in a logical fashion.

- `&&` will execute the next command, if the previous command executes.

- `||` will execute the next command, if the previous command fails to execute.

- `;` will always execute the next command.

Everything after `#` will be ignored and treated as a comment. Logical connectors without a preceding command will fail. Logical connectors without a following command will be ignored.

### I/O Redirection and Piping

RShell uses i/o redirection and piping

- `<` and `<<<` are used for input redirection. The latter case is used for redirecting from a string rather than a file

- `>` and `>>` are used for output redirection. The latter appends to the file rather than overwriting the file.

- `|` is piping and is used to allow different programs to communicate with one another.

### Changing Directories

RShell supports changing directories.

- `cd` takes you back to your home directory.

- `cd -` takes you back to a previous directory. If the previous directory is not set, it will fail.

- `cd [PATH]` will take you the specified path.

### Signals - Ctrl C and Ctrl Z

- `Ctrl C` will terminate your current shell process. If there are no existing child processes it will fail.

- `Ctrl Z` will suspend your current shell process. If there are no existing child processes it will fail.

- `fg` and `bg` are supported, but are implemented in a simplistic manner. `fg` will bring back only the most recent process that has been suspended. `bg` will send to the background the most recent process. There are input and output conflicts associated with the process being sent to the background!

## Known Bugs and Issues

- Everything after `#` will be treated as a comment regardless of its integrity in the command.
- `&` and `|` are treated as improper use of connectors if used without its complement.
- The use of `echo` will echo everything following it until a `&`, `|`, `;`, or `#` is reached.
- Any `'` will be included in the echo.
- Able to run multiple instances of RShell in itself.
- Usage of `"` must always be followed by another matching `"`
- Tabs are not treated as spaces and instead are treated as a character.
- Multiple I/O redirection without a following pipe will be truncated to the closest redirection such as `cat < file1 < file2 < file3` will be truncated to just `cat < file1`.
- Appending a flag following a file redirection will result in interpretation of the flag as a command such as `ls > file1 -a`. `-a` will be executed as a command instead of a flag. So input such as `ls > file1 cat file1 | sort` will result in undefined behavior, but in this case an infinite loop. It is important to define your flags before executing a command with redirection.
- String input redirection, `<<<`, will create a file named `NULL_TEMP.TXT` to store and read from. If the same file name exists, it will be overwritten and deleted!
- When multiple processes are suspended, and one process is brought back up with `fg` and terminated with `Ctrl C`, and another previously suspended process is brought back up with `fg` will be terminated immediately.
- `bg` is known to cause conflicting issues between RShell's input and the background processes' input.
- `fg` and `bg` may create zombie processes if used together on the same "chain" of commands. There are other conflicting issues between the two where `fg` may leave `bg` suspended and another `fg` prompt is needed to kill it.
- If no other processes are being ran, `Ctrl + C` and `Ctrl + Z` will prompt an error only after a command has been executed.

# ls Implementation

RShell includes a custom `ls` implementation. Currently it only supports the following flags
when used: `-a`, `-l`, and `-R'

It should also be noted that `ls` is an entirely separate program of its own and is NOT included in
the RShell itself as a command.

## Flag Usage

Flags must always be preceded by a `-` and can be combined into cases like `-alR` or `-l -aR` in any
order.

## Error Messages

In the case of of being unable to access certain files or directories, the name of the path and the
reason, as to why the system call has failed, will be outputted.

### Known Bugs and Issues

- Accessing certain files and directories, like `.dbus` and sockets like `SingletonLock`, may require root user permissions and may result in undefined behavior
- Directory names and folders ending with `/` will cause undefined behavior
- Outputting long file or directory names will result in undefined output formatting
- The recursive flag will list out files and directories in a level-order fashion, as opposed to the original `ls` implementation which is done in in-order
- Lists files in an order where dot files appear first in alphabetical order
- Any `-` without a following valid flag would simply execture regular `ls`
