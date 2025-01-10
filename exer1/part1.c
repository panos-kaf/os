#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

	char cc,output[256],*buffer;
	int count=0,i=0;
	buffer=malloc(1000000*sizeof(char));
	if(!buffer){
		printf("Error with buffer allocation\n"); 
		exit(EXIT_FAILURE);
	}
	
	if (argv[3]==NULL) {
		printf("Please provide a character to search for as an argument\n"); 
		exit(EXIT_FAILURE);
	}

	int read_fd=open(argv[1],O_RDONLY);
	if(read_fd==-1) {
		perror("Problem opening input file"); 
		exit(EXIT_FAILURE);
	}
	if(read(read_fd,buffer,1000000*sizeof(char)-1)==-1){
		perror("Problem reading input file"); 
		exit(EXIT_FAILURE);
	}
	close(read_fd);

	while(buffer[i++]!=0) if(buffer[i-1]==argv[3][0]) count++;

	sprintf(output,"The character '%c' appears %d times in file %s\n",argv[3][0],count,argv[1]);
	int write_fd=creat(argv[2],S_IWUSR | S_IRUSR);
	if(write_fd==-1) {
		perror("Problem opening output file"); 
		exit(EXIT_FAILURE);
	}
	if(write(write_fd,output,strlen(output))==-1){
		perror("Problem writing on output file"); 
		exit(EXIT_FAILURE);
	}
	close(write_fd);
	free(buffer);	
	exit(EXIT_SUCCESS);
}


