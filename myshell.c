#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

extern char ** environ;
char homePwd[PATH_MAX]; //String to store path to home directory
char * shellPath; //To store the shell path displayed in this shell and all child processes

//Function to get size of any char * [] array
int getArrSize(char * args[]){
  int i = 0;
  while(args[i]){
    i++;
  }
  return i;
}

//Function to clear the shell
void clr(){
  int i = 0;
  while(i < 32){ //prints 32 empty new lines
    printf("\n");
    i++;
  }
}

//Function to display the user manual
void help(){
  FILE * fptr;
  char ch;
  fptr = fopen("readme.txt", "r");
  if(fptr == NULL){
    printf("\nCannot open user manual\n");
    exit(1);
  }
  ch = fgetc(fptr);
  while(ch != EOF){
    printf("%c", ch);
    ch = fgetc(fptr);
  }
  printf("\n");
  fclose(fptr);
}

//Function to quit the shell
void quit(){
  exit(0); //terminate shell program
}

//Function to echo <message>
void echo(char ** argv){
  int i = 1; //Starting at index 1 of argv
  while(argv[i] != NULL){ //Keep printing until argv ends
    printf("%s ", argv[i]);
    i++;
  }
  printf("\n");
}

//Function to clean outout for output redirection
void cleanLine(char * args[], char * cleanargs[], int sizeOfArr){
  int i = 0, loop = 0;
  int inarrowPos = 0;
  int outarrowPos = 0;
  for(i = 0; i < sizeOfArr; i++){
    if(strcmp(args[i], "<") == 0){
      inarrowPos = i;
    }
    if((strcmp(args[i], ">") ==0) || strcmp(args[i], ">>") == 0){
      outarrowPos = i;
    }
  }
  if((inarrowPos != 0) && (outarrowPos != 0)){
    for(loop = inarrowPos; loop < outarrowPos; loop++){
      cleanargs[loop] = strdup(args[loop]);
    }
    cleanargs[loop] = '\0';
  } else if ((inarrowPos == 0) && (outarrowPos != 0)){
    for(loop = 0; loop < outarrowPos; loop++){
      cleanargs[loop] = strdup(args[loop]);
    }
    cleanargs[loop] = '\0';
  }
}

//Function to print contents of specified directory
void dir(char * args[]){
  DIR *dirp;
  struct dirent *ep;
  //Try to open directory
  if((dirp = opendir(args[1])) == NULL) { //If directory cannot be opened or does not exist
    fprintf(stderr, "opendir %s %s\n", args[1], strerror(errno));
  } else { //Print out each file in directory until pointer points to null
    while((ep = readdir(dirp)) != NULL){
      printf("%s\n", ep->d_name);
    }
  }  
  closedir(dirp); //close the directory
}

//Function to print all the environment variables
void env(){
  int i; 
  for(i = 0; environ[i] != NULL; i++){ //Iterate through environ array and print each variable and its value
    printf("%s\n", environ[i]);
  }
}

//Function to print the shell prompt - as PWD
void printPrompt(){
  char cwd[PATH_MAX]; //String to store PWD path for prompt
  if (getcwd(cwd, PATH_MAX) != NULL){ //If PWD is not NULL, then print PWD as prompt
    printf("%s>", cwd); //Add arrow for ~effect~
  } else { //If failed to get PWD, print error message
      perror("getcwd() error");
  }
}

//Function to parse user input into array of strings
int parseLine(char *line, char * parsed[]){
  char * str; //String to store current token
  int index = 0;
  str = strtok(line, " "); //Split line at space, store current string in str
  while (str != NULL){ //While end of line is not reached
    parsed[index++] = str; //Add current string to array
    str = strtok(NULL, " "); //Split line again at space
  }
  parsed[index] = '\0'; //terminate array
  return index;
}

//Function to read user input
char * readLine(){
  char *line = NULL; //String to store user input
  ssize_t bufsize = 0;
  getline(&line, &bufsize, stdin); //Get user input and store in line
  int len = strlen(line); //Get length of line
  if(line[len-1] == '\n'){ //If there is a newline character at the end of userinput
    line[len-1] = '\0'; //Change newline character to null to signal end of string
  }
  return line;
}

//Function to print line - for testing
void printLine(char *line){
  printf("\n%s\n", line);
}

//Function to print parsed input (or any char *[] array) - for testing
void printParsedArr(char ** argv){
  int position = 0;
  while(argv[position] != NULL){
    printf("%s\n", argv[position]);
    position++;
  }
}

//Function to change the SHELL variable in environment to this shell's path
void changeShellPath(){
  char cwd[PATH_MAX]; //String to store PWD
  getcwd(cwd, PATH_MAX); //Get PWD and store in cwd
  if((shellPath = malloc(strlen(cwd)+strlen("myshell")+1)) != NULL){ //Check to see if string space allocation succesful
    shellPath[0] = '\0'; //shellPath is set as empty
    strcat(shellPath, cwd); //Add old PWD to shellPath
    strcat(shellPath, "/myshell"); //Add this shell's name to shellpath
  } else {
    fprintf(stderr, "Malloc failed \n"); //If failed to allocate memory for shellPath, print error message
  }
  setenv("SHELL", shellPath, 1); //Update SHELL variable in environment
}

//Function to change the current working directory
void cd(char * args[]){ 
  //Check if only CD entered
  if(args[1] == NULL){
    chdir(homePwd); //Change current working directory to Home directory
    setenv("PWD", homePwd, 1); //Update PWD in environment to Home directory path
    return;
  }

  //Else create new path to the specified directory
  char cwd[PATH_MAX]; //String to store PWD
  getcwd(cwd, PATH_MAX); //Get PWD
  char * newCwd; //String to store the new path
  if((newCwd = malloc(strlen(cwd)+strlen(args[1])+2)) != NULL){ //Check is malloc succesfully allocated memory for newCwd
    newCwd[0] = '\0'; //Set newCwd as empty
    strcat(newCwd, cwd); //Add current PWD to newCwd
    strcat(newCwd, "/"); //Add a '/' character to newCwd
    strcat(newCwd, args[1]); //Add the specified directory name to newCwd
  } else { //If malloc failed to allocare memory, print error message
     fprintf(stderr, "Malloc failed \n");
  }
  //Try to change directory to specified directory
  if(chdir(newCwd) != 0){ //If failed to open specified directory
    perror(newCwd); //Print error message1
  } else { //Directory was opened succesfully
    setenv("PWD", newCwd, 1); //Update PWD to new PWD
  }
}

//Function to check if input has been redirected with '<'
//Returns 1 if true, 0 if false
int checkForInputRe(char * parsedArr[], int sizeOfArr){
  int i = 0;
  for(i = 0; i < sizeOfArr; i++){
    if(strncmp(parsedArr[i], "<", 1) == 0){ //Comparing parsed commandline array with '<'
      return 1;
    }
  }
  return -1;
}

//Function to check if output has been redirected with '>' or '>>'
//Returns 1 if true, 0 if false
int checkForOutputRe(char * parsedArr[], int sizeOfArr){
  int i = 0;
  for(i = 0; i < sizeOfArr; i++){
    if(strncmp(parsedArr[i], ">>", 2) == 0){ //Comparing parsed commandlind array with '>>' 
      return 2;
    } else if(strncmp(parsedArr[i], ">", 1) == 0){ //Comparing parsed commandline array with '>'
      return 3;
    }
  }
  return -1;
}

//Function to get filename that is to be used as input
char * getInputFile(char * parsedArr[], int sizeOfArr, char * filename){
  int i;
  for(i = 0; i < sizeOfArr; i++){
    if(strncmp(parsedArr[i], "<", 1) == 0){ //Comparing parsed commandline array with '<', if true
      filename = parsedArr[i+1];            //Will store the next string in array, which is the filename
    }
  }
  return filename;
}

//Function to get filename that is to be used as input
char * getOutputFile(char * parsedArr[], int sizeOfArr, char * filename){
  int i;
  for(i = 0; i < sizeOfArr; i++){
    if(strncmp(parsedArr[i], ">>", 2) == 0){       //Comparing parsed commandline array with '<', if true
      filename = parsedArr[i+1];                   //Will store the next string in array, which is the filename
    } else if(strncmp(parsedArr[i], ">", 1) == 0){ //Comparing parsed commandline array with '<', if true
      filename = parsedArr[i+1];                   //Will store the next string in array, which is the filename
    }
  }
  return filename;
}

//Function to redirect input, returns old file descriptor (oldfd) for restoring STDIN later
int redirectInputFile(char * infile){
  int oldfd = dup(STDIN_FILENO); //Save input oldfd
  int fd = open(infile, O_RDONLY); //Open file for input
  dup2(fd, STDIN_FILENO); //Redirect input to the opened file
  close(fd); //Close file
  return oldfd; 
}

//Function to redirect output, returns old file descriptor (oldfd) for restoring STDOUT later
//Takes in int as redirection type (redType), referring to '>' or '>>'
int redirectOutputFile(char * outfile, int redType){
  int oldfd = dup(STDOUT_FILENO); //Save output oldfd
  if(redType == 2){ //If redType = 2, redirection of output is of type '>>'
    int fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0666); //Open file for writing, it will be created if it does not exist
                                                                 //And for '>>', the output file is appeneded to if it does
                                                                 //This file will have read and write permissions, denoted by code 0666
    dup2(fd, STDOUT_FILENO); //Redirect output to the opened file
    close(fd); //Close file
  } else if(redType == 3){ //If redType = 2, redirection of output is of type '>'
    int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666); //Open file for writing, it will be created if it does not exist
                                                                //And for '>', the output file is truncated if it does
                                                                //This file will have read and write permissions, denoted by code 0666
    dup2(fd, STDOUT_FILENO); //Redirect output to the opened file
    close(fd); //Close file
  } else{ //No IO redirection specified
    printf("\nNo redirection specified\n");
  }
  return oldfd;
}

//Function to restore output to previous file descriptor after IO redirection
void restoreStdOutput(int oldfd){
  dup2(oldfd, 1); //Redirect input back to previous file descriptor
  close(oldfd);
}


//Function to restore input to previous file descriptor after IO redirection
void restoreStdInput(int oldfd){
  dup2(oldfd, 0); //Redirect input back to previous file descriptor
  close(oldfd);
}

//Function to check if user entered a built in command from batch file
int isBuiltInCmdWFile(char * parsedArr[], int sizeOfArr){
  //Compare command entered by user, which is parsedArr[0], against built in commands of shell
  //Matching built in command is executed
  if(strncmp(parsedArr[0], "cd", 2) == 0){
    cd(parsedArr);
    return 1;
  } else if(strncmp(parsedArr[0], "clr", 3) == 0){
    clr();
    return 1;
  } else if(strcmp(parsedArr[0], "dir") == 0){
    dir(parsedArr);
    return 1;
  } else if(strcmp(parsedArr[0], "env") == 0){
    env();
    return 1;
  } else if(strncmp(parsedArr[0], "echo", 4) == 0){
      echo(parsedArr);
    return 1;
  } else if(strncmp(parsedArr[0], "help", 4) == 0){
    help(); 
    return 1;
  } else if(strncmp(parsedArr[0], "quit", 4) == 0){
    quit(parsedArr);
    return 1;
  } else {
    return 0;
    }
  return 0;
}

//Function to check if user entered a built in command from commandline
int isBuiltInCmd(char * parsedArr[], int sizeOfArr){
  //For built in commands, output can be redirected for DIR, ENV, ECHO and HELP
  int outputRe = checkForOutputRe(parsedArr, sizeOfArr); //Check if user specified output redirection
  char * outfile; //String to store file name of output file
  if(outputRe == 2 || outputRe == 3){ //If outputRe = 2 or outputRe = 3, then user has specified output redirection
    outfile = getOutputFile(parsedArr, sizeOfArr, outfile); //Get file name of output file
  }
  
  //Compare command entered by user, which is parsedArr[0], against built in commands of shell
  //Matching built in command is executed
  if(strncmp(parsedArr[0], "cd", 2) == 0){
    cd(parsedArr);
    return 1;
  } else if(strncmp(parsedArr[0], "clr", 3) == 0){
    clr();
    return 1;
  } else if(strcmp(parsedArr[0], "dir") == 0){  
    
    if(outputRe == 2 || outputRe == 3){ //If output redirection needed
      int oldfd = redirectOutputFile(outfile, outputRe); //Redirect output to specified output file
                                                         //Save old file descriptor for later output restoration
      dir(parsedArr); //Execute command
      restoreStdOutput(oldfd); //Restore output
    } else { //If no output redirection needed
      dir(parsedArr);
    }
    return 1;
  } else if(strcmp(parsedArr[0], "env") == 0){
    
    if (outputRe == 2 || outputRe == 3){ //If output redirection needed
      int oldfd = redirectOutputFile(outfile, outputRe); //Redirect output to specified output file
                                                         //Save old file descriptor for later output restoration
      env(); //Execute command
      restoreStdOutput(oldfd); //Restore output
    } else { //If no output redirection needed
      env();
    }
    return 1;
  } else if(strncmp(parsedArr[0], "echo", 4) == 0){
    
    if(outputRe == 2 || outputRe == 3){ //If output redirection needed
      int oldfd = redirectOutputFile(outfile, outputRe); //Redirect output to specified output file
                                                         //Save old file descriptor for later output redirection
      echo(parsedArr); //Execute command
      restoreStdOutput(oldfd); //Restore output
    } else { //No output redirection needed
      echo(parsedArr);
    }
    return 1;
  } else if(strncmp(parsedArr[0], "help", 4) == 0){
      if(outputRe == 2 || outputRe == 3){ //If output redirection needed
        int oldfd = redirectOutputFile(outfile, outputRe); //Redirect output to specidied output file
                                                           //Save old file descriptor for later output redirection
        echo(parsedArr); //Execute command
        restoreStdOutput(oldfd); //Restore output
      } else { //No output redirection needed
        help();
      }
    return 1;
  } else if(strncmp(parsedArr[0], "quit", 4) == 0){
    quit(parsedArr);
    return 1;
  } else { //Command entered was not a built in one
    return 0;
  }
}

//Function to check if user specified piping in commandline
//Returns number of pipes in commandline
int checkForPipe(char * args[], int sizeOfArr){
  int i;
  int numPipes = 0;
  for(i = 0; i < sizeOfArr; i++){
    if(strcmp(args[i], "|") == 0){ //Go through commandline array and count number of pipes found
      numPipes++;
    }
  }
  return numPipes;
}

//Function to execute external commands
int executeCmd(char * args[], int sizeOfArr){
  int outputRe = checkForOutputRe(args, sizeOfArr); //Check if output redirection needed, returns -1 if not
  char * outfile; //To store file name of output file
  if(outputRe == 2 || outputRe == 3){ //If output redirection needed
    outfile = getOutputFile(args, sizeOfArr, outfile);
  }
  int inputRe = checkForInputRe(args, sizeOfArr); //Check if input redirection needed, returns -1 if not
  char * infile; //To store file name of input file
  if(inputRe == 1){ //Input redirection needed
    infile = getInputFile(args, sizeOfArr, infile);
  }

  //Check to see if command is specified as background process
  int isBackgroundProc; //Store value for whether it is background process
                        //1 for TRUE, 0 for FALSE
  int i = 0;
  char * p;
  while(args[i] != '\0'){  //Search parsed array for '&' char
    if((p = (strchr(args[i], '&'))) != NULL){ //If found
      args[i] = '\0'; //Remove the char from array of arguments
      isBackgroundProc = 1; //Return true for background process
    } else {
      isBackgroundProc = 0; //Return false for background process
    }
    i++;
  }

  pid_t pid;
  pid_t wpid;
  int status;
  
  if((pid = fork()) == 0){ //Forking successful, child process executes
    setenv("PARENT", shellPath, 1); //Set PARENT variable of child environment to the shell's path
    if((inputRe == 1) && (outputRe != -1)){ //If input and output redirection needed
      //Redirect input and output
	int oldIfd = redirectInputFile(infile);
	int oldOfd = redirectOutputFile(outfile, outputRe);
      //Execute command
	if(execvp(args[0], args) < 0){ //If execution failed
	  printf("%s: Command not found. \n", args[0]); //Print and exit
	  exit(1);
	}
	//Restore input and output
	restoreStdInput(oldIfd);
	restoreStdOutput(oldOfd);
    } else if((inputRe == 1) && (outputRe == -1)){ //If only input redirection needed
      //Redirect input
      int oldIfd = redirectInputFile(infile);
      //Execute
      if(execvp(args[0], args) < 0){ //If execution failed
	printf("%s: Command not found. \n", args[0]); //Print error and exit
	  exit(1);
	}
      //Restore inpt
	restoreStdInput(oldIfd);
    } else if ((inputRe == -1) && (outputRe != -1)){ //If only output redirection needed
      //Redirect output
      int oldOfd = redirectOutputFile(outfile, outputRe);
      //Execute
      if(execvp(args[0], args) < 0){ //If execution failed
	printf("%s: Command not found. \n", args[0]); //Print error and exit
	  exit(1);
	}
      //Restore output
	restoreStdOutput(oldOfd);	
    } else { //No IO redirection needed
      if(execvp(args[0], args) < 0){ //If execution failed
	printf("%s: Command not found. \n", args[0]); //Print error and exit
	  exit(1);
	}
    }
  } else if(pid < 0){ //If pid < 0, forking was unsuccessful
      printf("Forking unsuccessful");
      exit(1);
  } else {//If pid > 0 then it is parent process
      if(isBackgroundProc){ //If background process, return to commandline
        return 0;
      }

      do {//Keep checking to see if child process has terminated
	//Because here, pid  > 0, it specifies the process ID of a single child process for which status is requested
	wpid = waitpid(pid, &status, WUNTRACED); //Waitpid returns status of specified child process
	                                         //WUNTRACED ensures that the child process specified by pid, will have
	                                         //Its status reported even if its status has not been reported since
	                                         //It stopped
      } while(!WIFEXITED(status) && !WIFSIGNALED(status)); //While child process has not terminated
                                                           //WIFEXITED evaluates to non-zero value if status was returned
                                                           //For a child process that terminated normally
                                                           //WIFSIGNALED evaluates to non-zero value is status was returned
                                                           //For a child process that terminated due to the receipt of a
                                                           //Signal that was not caught
    }
    return 0;
}

//Function to check whether user entered file containing commandlines for shell
void executeWPipe(char * leftOfPipe[], char * rightTillNextPipe[]){
  int fd[2];
  pid_t pid1;
  pid_t pid2;
  int status1;
  int status2;
  
  if(pipe(fd) == -1){ //Pipe creation failed
    fprintf(stderr, "Pipe error: %s\n", strerror(errno)); //Print error and exit
    exit(1);
  }
  if((pid1 = fork()) == -1){ //Forking failed
    fprintf(stderr, "Fork error: %s\n", strerror(errno)); //Print error and exit
    exit(1);
  } else if (pid1 == 0){ //Forking was succesfull, child process executes
                         //This is the command to the left of the pipe
    setenv("PARENT", shellPath, 1); //Set PARENT variable of child environment to the shell's path
    dup2(fd[1], 1); //Change output to write end of pipe. 0 is read, 1 is write
    if(close(fd[1] == -1)){ //Failure to close file descriptor for output
      fprintf(stderr, "Error closing the WRITE side\n"); //Print error and exit
      exit(1);
    }
    close(fd[0]); //Close file descriptor for input
    //Execute command on left of pipe
    if(execvp(leftOfPipe[0], leftOfPipe) < 0){ //If execution failed
      printf("%s: Command not found.\n", leftOfPipe[0]); //Print error and exit
      exit(1);
    }
  } else { //Parent process
    if((pid2 = fork()) == -1){ //If forking unsuccesful
      fprintf(stderr, "Fork error: %s\n", strerror(errno)); //Print error and exit
      exit(1);
    } else if (pid2 == 0){ //Forking succesful, child process executes
                           //This is the command to the right of the pipe
      setenv("PARENT", shellPath, 1); //Set PARENT variable of child environment to the shell's path
      dup2(fd[0], 0); //Change input to the read end of pipe
      close(fd[0]); //Close file descriptor for input
      close(fd[1]); //Close file descriptor for output
      //Execute command on the right of pipe
      if(execvp(rightTillNextPipe[0], rightTillNextPipe) < 0){ //If execution failed
	printf("%s: Command not found.\n", rightTillNextPipe[0]); //Print error and exit
	exit(1);
      }
    } else { //Parent of second process
      //Close any unused file descriptors
      close(fd[0]);
      close(fd[1]);
      do { //Keep checking to see if the second child process has terminated
	//Because here, pid  > 0, it specifies the process ID of a single child process for which status is requested
	waitpid(pid2, &status2, WUNTRACED); //Waitpid returns status of specified child process
	                                    //WUNTRACED ensures that the child process specified by pid, will have
	                                    //Its status reported even if its status has not been reported since
	                                    //It stopped
      } while(!WIFEXITED(status2) && !WIFSIGNALED(status2)); //While child process has not terminated
                                                             //WIFEXITED evaluates to non-zero value if status was returned
                                                             //For a child process that terminated normally
                                                             //WIFSIGNALED evaluates to non-zero value is status was returned
                                                             //For a child process that terminated due to the receipt of a
                                                             //Signal that was not caught
    }
    //Close any unused file descriptors
    close(fd[0]);
    close(fd[1]);
    do { //Keep checking to see if the first child process has terminated
      waitpid(pid1, &status1, WUNTRACED);
    } while (!WIFEXITED(status1) && !WIFSIGNALED(status1)); 
  }
}

//Function to process pipes in commandline
void splitWPipe(char * args[], int sizeOfArr, char * leftPipeArgs[], char * rightPipeArgs[], char * rightTillNextPipe[], int pipesRemaining){
  int i; //Counter for iterating through commandline array
  for(i = 0; i < sizeOfArr; i++){ //Search commandline array until first occurence of a pipe
    if(strcmp(args[i], "|") == 0){; //If pipe found
      int positionOfPipe = i;       //Store index of pipe in commandline array
      int l;                       //Counter for iterating through array holding the command
                                   //(And any arguments for it) of the left of the pipe
      for (l = 0; l < positionOfPipe; l++){ //Starting from 0, go until the index of the
	                                    //First pipe found in commandline array
	leftPipeArgs[l] = strdup(args[l]); //Duplicate the string from the commandline at
	                                   //Index l to the array holding commands (and arugments)
	                                   //of the left of the pipe
	}
      leftPipeArgs[l] = '\0';              //After everything on the left of the pipe has been copied
                                           //To the left side array, add a '\0' to terminate the array
      
      int r = 0;               //Counter for iterating through array holding everything in the
                               //Commandline on the right of the pipe
      int rightOfPipe;         //Counter for iterating through the commandline array
        // rightOfPipe is set to the index of the first occurence of a pipe found in
        //Commandline array
        // rightOfPipe will iterate through the commandline array starting from
        //The occurence of the first pipe until the end of the commandline
	for(rightOfPipe = positionOfPipe+1; rightOfPipe < sizeOfArr; rightOfPipe++){
	  rightPipeArgs[r] = strdup(args[rightOfPipe]); //Iteratively copy everything on right of pipe
	                                                //In the commandline into the array holding the
	                                                //Remaining commandline after the first pipe
	  r++;  
	}
	rightPipeArgs[r] = '\0';           //After everything on the right of the pipe has been copied
	                                   //To the right side array, add a '\0' to terminate the array
	int sizeOfRight = getArrSize(rightPipeArgs); //Get size of the array holding the remaining
	                                             //Commandline on the right of the array
	int nextPipe; //To store location of next pipe in remaining commandline

	if(pipesRemaining == 1){ //If only one more pipe remains to be processed in commandline
	  int iter; //Counter to iterate through the array that will hold the command (and any arguments)
	            //On the immediate right of the pipe being currently processed
	  for(iter = 0; iter < sizeOfRight; iter++){ //Iter will go through the array containing the remaining
	                                             //Commandline on the right of the current pipe being processed
	                                             //Starting from 0 until the end of this array
	    rightTillNextPipe[iter] = strdup(rightPipeArgs[iter]); //Copy everything on the right of the current pipe
	                                                           //Being processed until the end of the remaining
	                                                           //commandline
	  }
	  rightTillNextPipe[iter] = '\0';            //After everything on the right of the pipe has been copied
	                                             //Add a '\0' to terminate the array
	  break;                                     //Break out of loop because no more pipes remain
	} else { //There are more than one pipes remaining to be processed in the remaining commandline
	  for(nextPipe = 0; nextPipe < sizeOfRight; nextPipe++){ //Next pipe will iterate through the array containing
	                                                         //The remaining commandline on the right of the pipe
	                                                         //Starting from 0 until the end of this array
	    if(strcmp(rightPipeArgs[nextPipe], "|") == 0){       //Search for first occurence of pipe in the remaining
	                                                         //Commandline
	      int positionOfNextPipe = nextPipe;                 //Store index of this pipe's occurence
	      int iter;                                          //Iter will go through the array containing the remaining
	                                                         //Commandline on the right of the current pipe being
	                                                         //Processed, starting from 0 until the location of the next
	                                                         //Pipe found in remaining commmandline
	      for(iter = 0; iter < positionOfNextPipe; iter++){
		rightTillNextPipe[iter] = strdup(rightPipeArgs[iter]); //Copy everything on the right of the current pipe
		                                                       //Being processed until the occurence of the next
		                                                       //Pipe in the remaining commandline
	      }
	      rightTillNextPipe[iter] = '\0';        //After everything on the right of the pipe has been copied
	                                             //Add '\0' to terminate the array
	      break;                                 //Break out of the loop because we are only looking for the
	                                             //Immediately next pipe
	    }
	  }
	}
	return; 
      }
    }
}

//Function to check if user entered a file containing commands for the shell
//Returns 1 for TRUE, 0 for FALSE
int isFileCmd(char * args[]){
  if(strncmp(args[0], "myshell", 7) == 0){
    return 1;
  }
  return 0;
}

//Function to find the number of commandlines found in file containing commands
int checkNumOfCmds(char * args[]){
  FILE * file;
  if((file = fopen(args[1], "r")) == NULL){
    perror("fopen source file");
    exit(1);
  }
  int i = 0; //Counter for number of commandlines in file
  if(file != NULL){
    char line [100];
    while(fgets(line, sizeof(line), file) != NULL){ //Get commandline from file
      int len = strlen(line);
      if(line[len-1] == '\n'){ //If end of the string contains a newline character
	line[len-1] = '\0'; //Replace it with a '\0' character to terminate string
      }
      i++; //Counter increments everytime we get a line from file
    }
  }
  fclose(file);
  return i; //Return number of commands found in batch file
}

//Function to extract commandslines from input file
//And execute those commandlines one by one
void getFileCommands(char * args[], int numOfCmds, int sizeOfArr){

  /* Opening the file and counting the number of characters in each */
  /* Commandline in the file and storing them in array for sizes of commands */
  /* Strings obtained from file had extra characters at the end */
  /* This is to find out the real character count of commandlines in file*/
    
  int sizesOfCmds[numOfCmds]; //Array to hold size in chars of each
                              //commandline in file
  FILE * f2;
  if((f2 = fopen(args[1], "r")) == NULL){
    perror("fopen source file");
    exit(1);
  }
  int j = 0;
  if(f2 != NULL){
    char line[100];
    while(fgets(line, sizeof(line), f2) != NULL){ //Get each commandline
      int len = strlen(line); //Get length of commandline
      sizesOfCmds[j] = len; //Store length of commands in command sizes array
      j++; //Move onto next command in file
    }
  }
  fclose(f2);

  /* Get commandlines from file one by one, remove extra characters at end*/
  /* Execute as built in or external command */
  /* Then move onto next command in file */
  
  int count;
  FILE * f3;
  if((f3 = fopen(args[1], "r")) == NULL){
    perror("fopen source file");
    exit(1);
  }
  for(count = 0; count < numOfCmds; count++){ //Loop only runs for the amount of commandlines in file
    
    char * cmdline; //To store single commandline from file
    
    size_t sizeOfCmd = sizesOfCmds[count]; //Count is specific to each commandline in the file  
                                           //Each commandline has its own length
                                           //Allocate that specific length to the buffer to store the
                                           //Commandline
    size_t length;                     
    cmdline = (char*)malloc(sizeOfCmd * sizeof(char));
    if(cmdline == NULL){ //If malloc failed, print error
      perror("Unable to locate commandline");
      exit(1);
    }
    length = getline(&cmdline, &sizeOfCmd, f3); //Get commandline from file
    if(count != numOfCmds-1){ //All commands except the last one have 2 extra
                              //Characters at the end
      char string[length];
      strcpy(string, cmdline);
      string[length-2] = '\0'; //Removing extra 2 characters at the end of
                               //Commandline string
      parseLine(string, args); //Parse the string into array
      
      if(!isBuiltInCmdWFile(args, sizeOfArr)){ //Check the array for built in commands
	                       //If built in commands then execute
	executeCmd(args, sizeOfArr);      //Else execute as external commands
      }
    } else { //The last command in the file has no extra characters, its fine
             //To parse it as it is
      parseLine(cmdline, args); //Parse the cmdline into array
      if(!isBuiltInCmdWFile(args, sizeOfArr)){  //Check the array for built in commands
                                //If built in commands then execute
        executeCmd(args, sizeOfArr);       //Else execute as external commands
      }
    }
  }
  exit(0);
}


int main(int argc, char **argv){

  getcwd(homePwd, PATH_MAX); //Store path to home directory
  changeShellPath(); //Change shell path to show path for this shell
  char * parsedArr[256]; //Array of pointers to store parsed user input
  char * line; //Line to store user input
  char * leftPipeArgs[256]; //For piped commands, stores everthing on left of pipe
  char * rightPipeArgs[256]; //For piped commands, stores everything on right of pipe
  char * rightTillNextPipe[256]; //For piped commands, stores everything on right of pipe
                                 //Till the next pipe OR end of commandline
                                 //This basically holds the first command on the right
                                 //Of the pipe and any arguments given for that command
  while(argv[0] != "quit"){ //While user does not enter "quit"
    
    printPrompt(); //Print the current working directory as a prompt
    line = readLine(); //Read user input and store in line
    int sizeOfParsedArr = parseLine(line, parsedArr); //Parse the line and store it in parsedArr
    int numPipes = checkForPipe(parsedArr, sizeOfParsedArr); //Check if commandline contains pipes
                                                             //Store number of pipes found
    if (numPipes > 0){ //If commandline contains pipe(s)
      int pipesRemaining = numPipes; //Store number of pipes remaining to be processed
      int count; //Counter for the amount of times we recursively execute on piped commandline
      //Split the command line based on where pipes are, this processes one pipe from the commandline
      splitWPipe(parsedArr, sizeOfParsedArr, leftPipeArgs, rightPipeArgs, rightTillNextPipe, pipesRemaining);
      pipesRemaining--; //Decrement number of pipes remaining to be processed
      int rightPipeArgsSize = getArrSize(rightPipeArgs); //Get size of commandline array to the right of the pipe
      executeWPipe(leftPipeArgs, rightPipeArgs); //Execute the commands on the left and immediate right of the pipe
                                                 //currently being processed
      for(count = 1; count < numPipes; count++){ //One pipe has already been processed and its commands executed
	                                         //If there are more pipes, the loop will run until we have
	                                         //Processed all pipes
	//Recursively call function to split and process the remaining commandline after the previous pipe
	splitWPipe(rightPipeArgs, rightPipeArgsSize, leftPipeArgs, rightPipeArgs, rightTillNextPipe, pipesRemaining);
	pipesRemaining--; //Decrement number of pipes remaining to be processed
       	executeWPipe(leftPipeArgs, rightPipeArgs); //Recursively execute the commands of the left and immediate right
	                                           //Of the next pipe found in the remaining commandline
      }
    } else {
      if(isFileCmd(parsedArr)){ //If user entered file containing commandlines for shell
       int numOfCmds = checkNumOfCmds(parsedArr); //Get the number of commands
                                                  //Found in input file
       getFileCommands(parsedArr, numOfCmds, sizeOfParsedArr); //Process file to get commands
                                                               //And execute those commands
      } else {
       if(!isBuiltInCmd(parsedArr, sizeOfParsedArr)){ //Check if user entered built in command
	                                              //And handle it
       executeCmd(parsedArr, sizeOfParsedArr);  //If not, execute external command
       }
      }
    }
}
}
