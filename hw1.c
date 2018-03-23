#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <signal.h>
#define READ 0
#define WRITE 1
static char* argv[100];
static char* cmd_left[50];
static char* cmd_right[50];
static int argc;

void parse(char* cmd);
//void handler(int sig);
void exec(char** arg_list, int type); 
void pipe_exec(char**arg_list, int i);

int main(){
	
	pid_t pid;
	char cmd[100];
	int flag = 0; //executed or not
	//char* argv[100]; //user command
	//int argc; //arg count
	
	while(1){
		printf(">");
		argc = 0;
		flag = 0;
		
		if(!(fgets(cmd, 99, stdin))) 
			break;
		if(strcmp(cmd, "exit\n")==0){
			while ((waitpid(-1, NULL, 0)) > 0); //ensure every child returns
			break;
		}
		if(cmd[strlen(cmd)-1]=='\n')
			cmd[strlen(cmd)-1]='\0';
		
		
		parse(cmd);
		
		
		for(int i=0;i<argc;++i){
			/*if( strcmp(argv[i], ">") == 0){
				flag =1;
				//I/O redirect
			}*/
			if(strcmp(argv[i], "|")==0){
				// pipe
				flag = 1;
				pipe_exec(argv, i);
				break;
			}
		}
		if( strcmp(argv[argc-1], "&") == 0){//non-blocking
			flag = 1;
			argv[argc-1] = '\0';
			exec(argv, 0);
		}
		if(flag==0)
			exec(argv, 1);
			
		
		/*for(int i=0;i<argc;++i){
			printf("%s\n", argv[i]);
		}
		printf("argc = %d\n", argc);*/
		
		//signal(SIGCHLD,handler);
	}
	
	
	return 0;
}
/*void handler(int sig){
	pid_t pid;
	int status;
	printf("signal triggered\n");
	printf("%c\n", argv[argc-1]);
	if(*argv[argc-1]=='&')
		pid = waitpid(0, &status, WNOHANG);
	else{
		printf("wait\n");
		wait(NULL);
	}
	return;
}*/
void exec(char** arg_list, int type){
	
	pid_t pid = fork();
	
	if(pid<0){
			printf("fork failed\n");
			exit(1);
		}
	else if(pid==0){//child process
		execvp(argv[0],argv);
		printf("execvp failed\n");
		exit(1);
	}
	
	else{//parent process
		//printf("type = %d\n", type);	
		if(type==0)
			waitpid(-1, NULL, WNOHANG);
		else if(type==1)
			while ((waitpid(-1, NULL, 0)) > 0);
		//printf("Child exited\n");
	}
	return;

}

void pipe_exec(char**arg_list, int i){
	int fd[2];
	if(pipe(fd)==-1){
		printf("Error opening pipe\n");
		exit(1);
	}
	pid_t pid_a, pid_b;
	
	//char* cmd_left[50];
	//char* cmd_right[50];
	for(int k=0;k<argc;++k){
		if(k < i)
			cmd_left[k] = arg_list[k];
		else if(k > i)
			cmd_right[k-i-1] = arg_list[k];
		else
			cmd_left[k] = '\0'; //set '\0' when k==i
	}
	cmd_right[argc-i-1] = '\0';
	/*printf("left cmd\n");
	for(int j=0;j<i;++j)
		printf("%s\n",cmd_left[j]);
	//printf("%s\n", cmd_left[1]);
	printf("right cmd\n");
	for(int j=0;j<argc-i-1;++j)
		printf("%s\n",cmd_right[j]);*/
	pid_a = fork();
	
	if(pid_a<0){
		printf("fork child_a failed\n");
		exit(1);
	}
	else if(pid_a==0){//child process on left
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		execvp(cmd_left[0],cmd_left);
		printf("execvp for chid_a failed\n");
		exit(1);
	}
	
	//parent process
	pid_b = fork();
	
	if(pid_b<0){
		printf("fork child_b failed\n");
		exit(1);
	}
	else if(pid_b==0){//child process on right
		close(fd[1]);
		dup2(fd[0], 0);
		close(fd[0]);
		execvp(cmd_right[0],cmd_right);
		printf("execvp for chid_b failed\n");
		exit(1);
	}
	
	else{ //parent process
		close(fd[0]);
		close(fd[1]);
		
		while ((waitpid(-1, NULL, 0)) > 0);
	}
	return;
	
}

void parse(char* cmd){
	char *token;
	token = strtok(cmd, " ");
	while(token!=NULL){
		argv[argc] = token;
		token = strtok(NULL, " ");
		++argc;
	}
	argv[argc] = NULL;
}
