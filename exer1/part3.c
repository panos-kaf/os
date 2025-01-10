#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define P 20

void handler(int sig){
	printf("\n%d processes are searching the file..\n",P);
}

int main(int argc,char *argv[])
{
	if(argv[3]==NULL) {
		printf("Please provide a character to search for\n"); 
		exit(EXIT_FAILURE);
	}
		
	struct sigaction sa;
	sa.sa_handler=handler;
	sigaction(SIGINT,&sa,NULL);
	
	int status;
	char *buffer,output[256];
	buffer=malloc(1000000*sizeof(char));
	
	int readfd=open(argv[1],O_RDONLY);
	if(readfd==-1){
		perror("Error while opening input file");
		exit(EXIT_FAILURE);
	}
	if(read(readfd,buffer,1000000*sizeof(char)-1)==-1){
		perror("Error while reading input file");
		exit(EXIT_FAILURE);
	} 
		
	int x=(strlen(buffer))/P;
	int pipefd[P][2]; // Create 2 file descriptors for each child
	//int sleep_dur=1600000/P;
		
	for(int i=0;i<P;i++){
		if(pipe(pipefd[i])==-1){
			perror("Error while establishing pipe..."); 
			exit(EXIT_FAILURE);
		}
		int id=fork();
		if (id==-1) {
			perror("Error while creating child process.."); 
			exit(EXIT_FAILURE);
		}
		else if (id==0){
			close(pipefd[i][0]);
			int count=0;
			for(int j=i*x;j<(i+1)*x;j++) 
				if(buffer[j]==argv[3][0]) count++;
			write(pipefd[i][1],&count,sizeof(int));
			//usleep(i*sleep_dur);
			//printf("PID #%d found '%c' %04d times\n",getpid(),argv[3][0],count);
			exit(EXIT_SUCCESS);
		}
	}
	
	for(int i=0;i<P;i++) wait(NULL);
	usleep(1000000);

	int res=0,total=0;
	for(int i=0;i<P;i++){ 
		close(pipefd[i][1]); 
		read(pipefd[i][0],&res,sizeof(int)); 
		total+=res;
	}
	printf("All workers finished execution..\nThe character '%c' appears %d times in file %s\n",argv[3][0],total,argv[1]);
	sprintf(output,"The character '%c' appears %d times in file %s\n",argv[3][0],total,argv[1]);
	
	int writefd=creat(argv[2], S_IWUSR | S_IRUSR);
	if(writefd==-1) {
		perror("Error while opening file"); 
		exit(EXIT_FAILURE);
	}
	if(write(writefd,output,strlen(output))==-1) {
		perror("Error while writing on output file"); 
		exit(EXIT_FAILURE);
	}
	close(writefd);
	
	free(buffer);
	exit(EXIT_SUCCESS);
}

