/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/*TODO header file for m(un)map*/
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

#define die(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

#define NEXT(x) ((x+1)%NPROCS)

void handler(int sig){
    reset_xterm_color(1);
    char newline='\n';
    write(1,&newline,sizeof(char));
    exit(EXIT_FAILURE);
}

/***************************
 * Compile-time parameters *
 ***************************/

sem_t *sema;

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;
	
/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

void compute_and_output_mandel_line(int fd, int line)
{
	/*
	 * A temporary array, used to hold color values for the line being drawn
	 */
	int color_val[x_chars];

	compute_mandel_line(line, color_val);
	output_mandel_line(fd, color_val);
}
/*
 * Create a shared memory area, usable by all descendants of the calling
 * process.
 */
void *create_shared_memory_area(unsigned int numbytes)
{
	int pages;
	void *addr;

	if (numbytes == 0) {
		fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
		exit(1);
	}

	/*
	 * Determine the number of pages needed, round up the requested number of
	 * pages
	 */
	pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

	/* Create a shared, anonymous mapping for this number of pages */
	/* TODO:  
		addr = mmap(...)
	*/
	int page_size = sysconf(_SC_PAGE_SIZE);
	addr = mmap(NULL, pages * page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

	return addr;
}

void destroy_shared_memory_area(void *addr, unsigned int numbytes) {
	int pages;

	if (numbytes == 0) {
		fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
		exit(1);
	}

	/*
	 * Determine the number of pages needed, round up the requested number of
	 * pages
	 */
	pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

	if (munmap(addr, pages * sysconf(_SC_PAGE_SIZE)) == -1) {
		perror("destroy_shared_memory_area: munmap failed");
		exit(1);
	}
}

void mandel_with_procs(int fd, int proc, int NPROCS){
	
	int color_val[x_chars];

	for(int i=proc;i<y_chars;i+=NPROCS){
		compute_mandel_line(i, color_val);
		if(sem_wait(&sema[proc]))
			die("sem_wait");
		output_mandel_line(fd, color_val);
		if(sem_post(&sema[NEXT(i)]))
			die("sem_post");
	}
}


int main(int argc,char** argv)
{
    	int NPROCS;
	if(argc<2){
		printf("\033[31mNPROCS not provided, assuming 1.\nUsage: %s [NPROCS]\n",argv[0]);
		reset_xterm_color(1);
		NPROCS=1;
	}
	else NPROCS=atoi(argv[1]);
	if(NPROCS<=0){
	      	printf("\033[31m" "NPROCS must be a positive value\n");
       		reset_xterm_color(1);
		exit(1);
	}	

	struct sigaction sa;
	sa.sa_handler=handler;
	sigaction(SIGINT,&sa,NULL); 

	sema = create_shared_memory_area(NPROCS*sizeof(sem_t));
	if(sem_init(&sema[0],1,1))
		die("sema_init");
	for(int i=1;i<NPROCS;i++){
		if(sem_init(&sema[i],1,0))
			die("sema_init");
	}

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	/*
	 * draw the Mandelbrot Set, one line at a time.
	 * Output is sent to file descriptor '1', i.e., standard output.
	 */
	for (int i=0;i<NPROCS;i++){
    		pid_t id = fork();
    		if(id==-1) 
		    die("fork");
		else if(id==0){
		    mandel_with_procs(1,i,NPROCS);
		    exit(0);
		}
	}
	for(int i=0;i<NPROCS;i++)
	    wait(NULL);
	printf("\033[31m" "All processes have finished.\n");
	reset_xterm_color(1);
	for(int i=0;i<NPROCS;i++)
	    sem_destroy(&sema[i]);
	destroy_shared_memory_area(sema, NPROCS*sizeof(sem_t));
	return 0;
}
