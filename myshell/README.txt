2018101008
Shivam Mangale

*No empty commands allowed.(No enter)
*For pinfo and part 5, run it on ubuntu.

The Shell works like a more basic version of mainstream shells like bash. It can run various commands like echo, cd, pwd, ls. For added functionality there's a pinfo function included which prints the process id, process status, memory and executable path. Process can also be run in the background and in a single line multiple commands can be executed.

*Enter when given prompt;
*Wherever the shell is called, that directory is the HOME == "~" directory for the shell.

Most of the commands which can be executed in bash can be executed here. To give multiple commands in a single input, separate each command with ';'.
the 'ls' command can handle '-l' and '-a' flags, whereas pwd, cd and echo don't support any flags. To run processes in the background add a '&' at the end of the command. All processes except ls, cd, pwd and echo can be run on the background. There is an added process called pinfo which prints information about the current process if no argument is given or the process with the specific pid which is given as an argument along with the 'pinfo' command.

Will not work for wrong/invalid commands.
Will not work for invalid directory inputs for cd and ls.
If input directory doesnt exist ls will show directory doesnt exist.
For cd, null activity will happen.That is nothing will happen.

Runs ls,cd,pwd,pinfo,set,unset,overkill.
Runs jobs,kjobs,fg,bg.
Runs other functions by execl call.
Inputs are as given in the assignment pdf.


Implemented up arrow command (bonus q1).