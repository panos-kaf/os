#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]){

	if (argv[3]==NULL) {
		printf("Please provide a character to search for as an argument\n"); 
		exit(EXIT_FAILURE);
	}
	
	printf("pid #%d: opening read and write files...\n",getpid());

	char output[256],*buffer;
	int count=0,i=0;
	buffer=malloc(1000000*sizeof(char));
	if(!buffer){
		printf("Error with buffer allocation\n"); 
		exit(EXIT_FAILURE);
	}
	
	int read_fd=open(argv[1],O_RDONLY);
	if(read_fd==-1) {
		printf("Problem reading input file\n"); 
		exit(EXIT_FAILURE);
	}
	
	read(read_fd,buffer,1000000*sizeof(char)-1);
	close(read_fd);
	int write_fd=creat(argv[2],S_IWUSR | S_IRUSR);

	int pid=fork();
	
	if(pid==-1) {
		printf("Error while creating child process..\n");
		exit(EXIT_FAILURE);
	}
	
	int status,exstat;
	
	if(pid==0){
		while(buffer[i++]!=0) if(buffer[i-1]==argv[3][0]) count++; 
		printf("pid #%d: encountered the character %d times..\n",getpid(),count);
		sprintf(output,"The character '%c' appears %d times in file %s\n",argv[3][0],count,argv[1]);
		write(write_fd,output,strlen(output));
		exit(count);
	}
	else{
		waitpid(pid,&status,0);
		if (WIFEXITED(status)){
			exstat=WEXITSTATUS(status); 
			printf("pid #%d: child process (#%d) was successfully killed..\n",getpid(),pid);
		}
		else if (WIFSIGNALED(status)){
			exstat=WTERMSIG(status); 
			printf("something went wrong with process #%d (Error #%d)\n",pid,exstat);
			exit(EXIT_FAILURE);
		}
		
		close(write_fd);
		free(buffer);
		printf("pid #%d: finished writing, terminating..\n",getpid());
		exit(EXIT_SUCCESS);
	}
}
