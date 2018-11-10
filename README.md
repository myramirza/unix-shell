# Simple UNIX Shell

Operator's Manual:

This is an implementation of a simple UNIX shell. You can type commands into it
and it will execute those commands.

For those unfamiliar with UNIX shells, there is a specific syntax for these
commands.
The prompt on the screen is the name of the file directory you are currently in.

Eg. linux1/home>

Once you type in a command, you must press enter for it to process. The shell will then read the command
and try to execute it. This command can fail to execute if:
1) There is no command of the sort you have typed.
2) The files you have specified in your command line cannot be found.
3) The syntax of the command is not recognizeable.

Given these restraints, it is important to type commands in exactly the way the shell expects them.
In this shell, if your command has any arguments for it, they should be of the form:

"command [arg1] [arg2] ... [argN]"

Eg. echo hello world

The first word in this line is the command you want to execute. The second, third and any further words
are going to be arguments that will be passed to the command.

Not every command requires arguments.

Eg. the command "ls" can be typed in without any arguments and when entered into the shell, it lists all
the files in the current directory.

Generally the shell will take input from the keyboard as its standard input and will display its output
on the screen as its standard outout. However you can redirect the input of your program from a textfile
if you type a "<" after your command and before the name of a file.

Eg. cmdname < inputfile.txt

You can also redirect the output of your command to a specific file if you type ">" or ">>" after your
command and its arguments.

Eg. echo hello world >> outfile.txt

There are two ways to redirect your output to a textfile this way.
If you type ">", it will create the file if it doesn't exist and truncate it if it does.
If you type ">>", it will create the file if it doesn't exist and append to it if it does.

You can even redirect input and output in the same commandline.

Eg. cmdname arg1 < inputfile.txt > output.txt

You can also run programs in the background while you use the shell to execute other programs.
To do this, you can add a "&" at the end of your command after a space.

Eg. ls &

Entering this will execute the command ls and immediately return the shell to the commandline for further input.

You can also redirect the input and output of several commands using pipes. You can do this by adding a "|"
between your commands.

Eg. cat out.txt | wc -l

This commandline will take the output of the command cat out.txt and use it as the input for the command
wc with its argument -l.
You can also pipe any number of commands together.

Eg. cmd1 | cmd2 | ... | cmdn

The program environment is a list of variables that have values assigned to them.

Eg. the variable HOME will have a value that is the path to the directory where the shell was executed from.

Eg. the variable PWD will have a value that is the path to the current directory you are in

Built in commands and what they do:

"cd" : change the current working directory to home directory

"cd <directory>" : change the working directory to  <directory>

"clr" : clear the screen

"dir <directory>" : list the contents of <directory>

"env" : list all environment variables

"echo <message>" : print <message>

"help" : display the user manual

"quit" : quit the shell