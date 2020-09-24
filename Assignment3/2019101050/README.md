#### Some Notes 
- Max length of string commands is 5000 characters (can be changed in headers.h)
- Memory in pinfo is shown in KB
- Not implemented "and" and "or" operation for commands
- Background commands syntax is like bash eg cmd1 & cmd2 & cmd3
- Tested on Ubuntu 18.04 and fedora 30
- /proc and <signal.h> are supported
- CTRL-D / quit command are used to exit the shell safely
- CTRL-C / CTRL-Z signal are blocked by shell
- Redirection handled like bash (no multi io)
- Piping works too


#### Brief documentation
- shell.c 
    - contains the main while loop
    - has some signal handlers
    - prints the cwd and hostname 
- parser.c
    - breaks down the input and parses it as required
    - supports &, ;, spaces, <, >, >> parsing
- ls.c
    - contains implementation for ls command
- pinfo.c 
    - contains implementation for pinfo command
- nightswatch.c 
    - contains implementation for nightswatch command 
    - also has implementation for handling raw mode which is required for nightswatch
- zombie_killer.c 
    - contains implementation for for handling children processes which have become zombies
- util.c 
    - contains implementation for frequently used functions like handling ~, min, max, color output etc
    - contains implementation for cd and echo too as the implementation is trivial
    - cd supports - (previous working directory)
- headers.h
    - contains most commonly used headers
- process_maker.c
    - handles creating child processes in both background and foreground to run different executables 
    - Also takes care of job related things, like adding & removing jobs
- history_handler.c
    - contains implementation for the history command which stores upto 20 commands
- env_var.c
    - implements add, remove, and getting environment variables

#### Self implemented commands (shell builtins)
- echo
- pwd
- ls -[la]
- exit
- clear
- pinfo
- history
- nightswatch
- getenv/setenv/unsetenv
- overkill
- jobs
- kjob 
- bg/fg


#### To Run
- make
- ./s

#### To Debug
- ./s2.sh

coded on mac with the help of parallels for Linux VM

