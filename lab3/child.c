#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>   

char * file_mmf_global;
int i_global = 0;

void writer(){
    msync(file_mmf_global, 500*sizeof(int), MS_SYNC| MS_INVALIDATE);
    int c;
    for (int i = i_global ; i < 500*sizeof(int); i++) {
        // printf("ЯЯ здесьььuuuuuuuuuu\n");
        c = file_mmf_global[i];
		if ((c != 'a') && (c != 'e') && (c != 'i') && (c != 'u') && (c != 'y') && (c != 'o') && 
		(c != 'A') && (c != 'E') && (c != 'I') && (c != 'U') && (c != 'Y') && (c != 'O')) {
			write(fileno(stdout), &c, sizeof(char));
            if (c == '\n'){
                i_global = i + 1;
                break;
            }
		}
	}	
}

void quit(){
    exit(0);
}

int main (int argc, const char *argv[]){
    int a = atoi(argv[1]);
	int out = 0;
	if (a == 1) {
		out = open("first_mmf.txt", O_RDWR);
	}
	else {
		out = open("second_mmf.txt", O_RDWR);
	}
	char *file_mmf =  mmap(NULL, 500*sizeof(int),  PROT_WRITE |PROT_READ , MAP_SHARED ,out,0);
	if (file_mmf == NULL) {
		perror ("Ошибка mmap\n");
	}

    file_mmf_global = file_mmf;
	// int c;
    // printf("ЯЯ здесььь\n");
	// for (int i = 0 ; i < 500*sizeof(int); i++) {
    //     msync(file_mmf, 500*sizeof(int), MS_SYNC| MS_INVALIDATE);
    //     // printf("ЯЯ здесьььuuuuuuuuuu\n");
    //     c = file_mmf[i];
	// 	if ((c != 'a') && (c != 'e') && (c != 'i') && (c != 'u') && (c != 'y') && (c != 'o') && 
	// 	(c != 'A') && (c != 'E') && (c != 'I') && (c != 'U') && (c != 'Y') && (c != 'O')) {
	// 		write(fileno(stdout), &c, sizeof(char));
	// 	}
	// }	
	// close(fileno(stdin));
    signal (SIGUSR1, writer);
	signal (SIGUSR2, quit);
	while (true);
}