#### Assumptions
- Max length of string commands is 5000 characters (can be changed in headers.h)
- Memory in pinfo is shown in KB
- Done all bonus questions too
- <Termios.h> is allowed for raw mode
- Tested on Ubuntu 18.04 and fedora 30
- /proc and <signal.h> are supported
- sigint / exit are used to exit the shell safely


#### Brief documentation
- shell.c 
    - contains the main while loop
    - reads and parses inputs and sends it to correct handlers
    - echo and pwd are handled by this file since the implementation is trivial
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
- headers.h
    - contains most commonly used headers
- process_maker.c
    - handles creating child processes in both background and foreground to run different executables 
- history_handler.c
    - contains implementation for the history command

#### Self implemented commands
- echo
- pwd
- ls -[la]
- exit
- clear
- pinfo
- history
- nightswatch

#### To Run
- make
- ./shell

coded on mac with the help of parallels for Linux VM

