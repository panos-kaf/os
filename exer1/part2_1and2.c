#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>


int main(){
	char *x;
	int id=fork();
	if(id==0){
		x="child";
		printf("\nHello world! Im a child process with pid #%d,\nmy parent's pid is #%d.\n\n",getpid(),getppid());
	}	
	else if(id==-1) 
		printf("Error while creating child process..\n");
	else{
	        wait(0);
		x="parent";
		printf("This is the parent of the process with pid #%d.\n\n",id);
	}
	printf("The process with pid #%d says: %s\n\n",getpid(),x);
	exit(0);
}



