# Shell (Part 2)
## Additional features
- Support for I/O redirection and piping. 
- For I/O redirection in different files, string tokens were checked for <, > or >> symbol(s) and corresponding flags were set. Based on the flags, the appropriate file is opened in the required mode and standard file descriptors (STDIN_FILENO and STDOUT_FILENO) are modified.
- ```jobs```, ```sig```, ```fg```, ```bg``` commands that access (linked) list of background processes.
- Signal handling (^C, ^D, ^Z).

## Piping
- Uses pipe() sytem call in a for loop to support multiple piping of commands.
- Commands on either side of | symbol are tokenized appropriately and stored in a 2D array of strings.
- I/O redirection is similar to that implemented for other files.

## Signals
- ctrl+C terminates foreground processes and has no other effect.
- ctrl+Z suspends (stops) foreground process and gives control back to the main shell program.
- ctrl+D exits the shell.

## Descriptions of files 
### main.c
- Uses input.c instead of fgets, so that ctrl+D functionality can be implemented.
- Tokenizes input by semicolons, whitespace and '|'.
- Calls built in as well as system functions and passes the tokens.
- Each valid command is executed by a separate function, defined in a separate file.
- Separate handling for commands with piping and without piping

### input.c
- Gets characters from STDIN and puts it in a buffer until newline or EOF (end of file or ctrl+D) character is encountered.
- By returning the appropriate integer value, main() function can determine whether error occurred, shell program needs to exit or input was accepted correctly.

### echo.c
- Prints tokens into STDOUT (default) or other file stream.
- Supports I/O redirection like bash.

### pwd.c
- Sets the cwd (current working directory) string to the path name of the working directory.
- Prints the present working directory, considering the home directory to be the directory from which the shell is invoked.
- Support for I/O redirection is implemented. 

### cd.c
- Accepts 1 argument as the target directory.
- Changes the current working directory to the argument, if possible.
- '-' flag changes to previous working directory while '~' flag (or no flag or argument) changes the directory to home directory. '.' flag signifies a change to the same directory (or no directory change) while '..' flag indicates a change into the parent directory.
- Changes shell prompt accordingly. If path of target directory is the home path or if the target directory is under the home directory, the home path is replaced by the '~' symbol in the shell prompt.

### ls.c
- Can accept '-l' and '-a' flags.
- Also accepts multiple arguments (directory/file paths)
- '-l' flag lists files in long listing format. If the listed files were last modified in a year other than the current year, the year will be specified in place of time (i.e., in place of 'hr:min').
- Displays files/directories one below the other.
- Support for I/O redirection is implemented. 

### pinfo.c
- Displays process information like pid (process id), status of process, memory space, executable path for the process.
- Accesses files and directories under /proc folder to obtain process information.
- Checks if process is running in foreground or background.
- Support for I/O redirection is implemented. 

### syscommands.c
- Executes other commands to run applications, spawn processes, etc. using execvp() function.
- Can run processes in foreground or background (add '&' at the end of command for background process).
- Prints process id and exit status of background process and uses fork() twice to implement this functionality.
- Dynamically allocates storage for tokens for the execvp() function.

### repeat.c
- Exeutes a command multiple times by calling the appropriate function.

### jobs.c
- Uses a linked list of background jobs to display them along with their process id and state (running or stopped).
- Has -r and -s flags for displaying "Running" and "Stopped" jobs respectively.

### sig.c
- Sends a signal to a background process by accepting job number and signal number from the user.

### makefile
- Compiles all .c files using gcc with -Wall compile flag (gcc -Wall *.c).

## Error handling
- Error checks are coded into all files to handle different situations.
- Format checks for commands have been implemented. 
