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

#include <errno.h>
#include <pthread.h> 
#include <semaphore.h>

#define perror_pthread(ret,msg) do{errno=ret; perror(msg);}while(0)

#define NEXT(x) ((x+1)%NTHREADS)

sem_t *sema;
int NTHREADS;
struct mandel_args{
	int fd;
	int thread_id;
};


#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

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
	
	char point ='%';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);	}
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


void* threaded_lines(void* thread_args){
	struct mandel_args* args=thread_args;
	int id=args->thread_id;
	char* t_id=malloc(12*sizeof(char));
	sprintf(t_id,"#%d :",id);
	for(int i=id;i<y_chars;i+=NTHREADS){
		sem_wait(&sema[id]);				//the sema of the 1st thread becomes 0
		//write(args->fd,&t_id,4);
		compute_and_output_mandel_line(args->fd,i);	//and after calling the mandel function
		sem_post(&sema[NEXT(id)]);			//it unlocks the next thread's sema
		}
	return NULL;
}

int main(int argc,char** argv)
{

	if(argc<2){
		printf("Usage: %s [NTHREADS]\n",argv[0]);
		exit(1);
	}
	NTHREADS=atoi(argv[1]);
	if(NTHREADS<=0){
	      	printf("NTHREADS must be a positive value\n");
       		exit(1);
	}	

	sema=malloc(NTHREADS*sizeof(sem_t));
	pthread_t thread[NTHREADS];
	int ret;
	sem_init(&sema[0],0,1);		//Sema for 1st thread is set to 1
	for(int i=1;i<NTHREADS;i++)	//Semas for the rest are set to 0
		sem_init(&sema[i],0,0);	//so that they have to wait for thread #1 to post

	struct mandel_args args[NTHREADS];

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

	/*
	 * draw the Mandelbrot Set, one line at a time.
	 * Output is sent to file descriptor '1', i.e., standard output.
	 */
	for (int i=0;i<NTHREADS;i++){
		args[i].fd=1;
		args[i].thread_id=i;

		ret=pthread_create(&thread[i],NULL,threaded_lines,(void*)&args[i]);
		if(ret){
			perror_pthread(ret,"pthread_create error");
			exit(1);
		}
	}

	for (int i=0;i<NTHREADS;i++){
		ret=pthread_join(thread[i],NULL);
		if(ret)
			perror_pthread(ret,"pthread_join error");
	}

	reset_xterm_color(1);
	return 0;
}
