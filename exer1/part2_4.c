#include <unistd.h>
#include <stdlib.h>

int main(){
	char *args[]={"charcount","input","output","s","NULL"};
	execv("./charcount",args);
	exit(0);
}
