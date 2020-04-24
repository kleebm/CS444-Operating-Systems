#include "shtypes.h"
#include "syscalls.h"
#include "io.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

extern char** mem[10][50];

int handle_builtin(struct cmd *c) /* c is the pointer to a cmd struct */
{
  /* already written : make sure c parsed properly and if not return 0 */
  if (!c)
    return 0;

	for(int i= 9; i>0;i--){
		strcpy(mem[i],mem[i-1]);
	}
	
	int count=0;	
	while(1){
		if(c->exec.argv[count]){
			strcpy(mem[0],c->exec.argv[count]);
			count++;
		}
		else
			break;
	}


  /* already written : tp is the type of the command, like Redir, background, 
  exec, list, or pipe (shtypes.h) */
  switch (c->tp) /* tp is the field out of the struct that we are pointing to (using the ->) */
  {
  /* Note : only if c is an exec then it could be a built-in */
  case EXEC: // Note : EXEC is an enum (shtypes.h)
    /* echo command already written */
    /* if comparing c->exec.argv[0] and "echo" gives 0 then they are 
    equal and you loop over the other arguments and print them out */


    if (strcmp(c->exec.argv[0], "echo") == 0) 
    {
      for (int i = 1; ; i++)
        if (c->exec.argv[i])
          printf("%s ", c->exec.argv[i]);
        else
          break;
      printf("\n");
      return 1;
	  }
    /* exit command already written */ 
    else if (strcmp(c->exec.argv[0], "exit") == 0) 
    {
		  int code = 0;
		  if(c->exec.argv[1]) {
			code = atoi(c->exec.argv[1]);
		}
		exit(code); /* kills your process and returns code */
    } 

    // TODO for students: implement cd
    //DONE
    else if(strcmp(c->exec.argv[0], "cd") == 0)
    {
	if(c->exec.argv[1] == NULL)
	{
		char cwd[1024];
		getcwd(cwd, sizeof(cwd));
		printf("Workdir: %s\n", cwd);
	}

	else 
	{
		chdir(c->exec.argv[1]);
		//printf("Did it work? %d \n", err);	
		//printf("Changed to directory: %s \n", c->exec.argv[1]);
	}	
	return 1;
    }

    // TODO for students: implement pwd
    //DONE
    else if(strcmp(c->exec.argv[0], "pwd") == 0)
    {
 	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("Workdir: %s\n", cwd);
	//printf("Filepath: %s/%s\n", cwd, __FILE__);
    	return 1;
    }

    // TODO for students: implement jobs
    //
    else if(strcmp(c->exec.argv[0], "jobs") == 0)
    {
	/*{
    	struct task_struct *task;
    	for_each_process(task)
    	{
    	printk("%s [%d]\n",task->comm , task->pid);
    	}

	//printf("Need to implement jobs \n");*/
	//return !fnmatch("[1-9]*", "/proc/"->d_name, 0);    
    }

    // TODO for students: implement history
    //
    else if(strcmp(c->exec.argv[0], "history") == 0)
    {
	for(int i = 0;i<10;i++){
		printf("%d: %s \n",i, mem[i]);
	}
    	return 1;
    }

    // TODO for students: implement kill
    // 
    else if(strcmp(c->exec.argv[0], "kill") == 0)
    {
	if(c->exec.argv[1] > 0){
		printf("Killing process ID: %s \n", c->exec.argv[1]);
		kill((int)c->exec.argv[1], SIGKILL);
	}
	else{	
		printf("Need to give a process ID to kill something \n");
	}
	return 1;
    }

    // TODO for students: implement help
    //DONE
    else if(strcmp(c->exec.argv[0], "help") == 0)
    {
	//add options to here if and when i add them
	printf("Here are a list of built-in commands for this shell: \n");
	printf("pwd: provides the current working directory \n");
	printf("cd: cd .. will move you one up and cd PATH will change your current directory to the PATH specified \n");
	printf("jobs: will print a list of all the current jobs running \n");
	printf("history: will list the previous 10 commands used. Can use !n to call the nth command again \n");
	printf("exit: will exit the program \n");
	printf("echo n: will print out whatever n is \n");
	printf("kill n: will terminate process with id n \n");
	printf("clear: will clear the screen from all previous commands \n");
	printf("help: will print a list of built-in commands, but youre already here! \n");
    	return 1;
    }
    
    // TODO for students: implement clear
    //DONE
    else if(strcmp(c->exec.argv[0], "clear") == 0)
    {
	system("clear");
	return 1;
    }
    else {
      return 0;
    }
  default:
    return 0; //Note : returns 0 when you call handle_builtin in main
  }
}

/* Most of your implementation needs to be in here, so a description of this
 * function is in order:
 *
 * int exec_cmd(struct cmd *c)
 *
 * Executes a command structure. See shtypes.h for the definition of the
 * `struct cmd` structure.
 *
 * The return value is the exit code of a subprocess, if a subprocess was
 * executed and waited for. (Id est, it should be the low byte of the return
 * value from that program's main() function.) If no return code is collected,
 * the return value is 0.
 *
 * For pipes and lists, the return value is the exit code of the rightmost
 * process.
 *
 * The function does not change the assignment of file descriptors of the
 * current process--but it may fork new processes and change their file
 * descriptors. On return, the shell is expected to remain connected to
 * its usual controlling terminal (via stdin, stdout, and stderr).
 *
 * This will not be called for builtins (values for which the function above
 * returns a nonzero value).
 */
int exec_cmd(struct cmd *c)
{
  /* don't try to execute a command if parsing failed. */
  if (!c)
    return -1;

  int child;

  switch (c->tp)
  {
  case EXEC:
	{
	pid_t child_pid;
  	int child_status;
	child_pid = fork();
	if(child_pid == 0){
		//This is the child process
		execvp(c->exec.argv[0],c->exec.argv);
		//If command does not work print this
		printf("Unkown command \n");
		exit(0);
	}

	else{
		//do{		
		pid_t tpid = wait(&child_status);
		//}while(tpid != child_pid );*/
		return child_status;	
	}
  	  
	break;
    }
  case PIPE:
	/* TODO for students: Run *two* commands, but with the stdout chained to the stdin
	 * of the next command. How? Use the syscall pipe: */
	/* At this point, pipefds[0] is a read-only FD, and pipefds[1] is a
		 * write-only FD. Make *absolutely sure* you close all instances of
		 * pipefds[1] in processes that aren't using it, or the reading child
		 * will never know when it needs to terminate! */
	{
		int pipefds[2];
		pipe(pipefds);
		pid_t child;
		if((child = fork()) == -1){
			perror(pipefds[0]);
			exit(0);
		}
		if(child == 0){
			close(pipefds[0]);
			exit(0);
		}
		else{
			close(pipefds[1]);
		}

		
	}
    break;
	
  case LIST:
	/* The LIST case is already written */
	exec_cmd(c->list.first);
	return exec_cmd(c->list.next);
    break;

  case REDIR:
   //You need to use the fields in the c->redir struct here; fd is the file 
   //descriptor you are going to redirect with (STDIN or STDOUR), the path 
   //is the file you need to open for redirecting, the mode is how to open
   //the file you are going to use, and the cmd field is what command to run
    break;

  case BACK:
	/* TODO for students: This should be easy--what's something you can remove from EXEC to have
	 * this function return before the child exits? */
    break;

  default:
    fatal("BUG: exec_cmd unknown command type\n");
  }
}
