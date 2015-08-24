the code files bathroom.c, enter.c and defs.h need to be put in a directory on a linux machine along witht he makefile.
type 'make' to compile.
bathroom is invoked by bathroom <size> [<interval>] 
enter is invoked by enter <w|m|W|M> <time>

to end the bathroom process, do a 'ctrl+c' on the terminal. it clears all shared data files (room.txt). the bathroom process does not recognize the bathroom stop command.