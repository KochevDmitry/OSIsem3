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

char* get_fileaname() {
    int len = 0;
    int capacity = 1;
    char *s = (char*) malloc(sizeof(char));
    char c = getchar();
    while (c != '\n') {
        s[(len)++] = c;
        if (len >= capacity) {
            capacity *= 2;
            s = (char*) realloc(s, capacity * sizeof(char));
        }
		if (capacity > 256) {
            s = NULL;
            return s;
		}
        c = getchar();
    }
    s[len] = '\0';
    return s;
}

int main (){
	enum {
		READ = 0,
		WRITE = 1
	};

	char * first_file = NULL;
	first_file = get_fileaname();
	if (first_file == NULL) {
		perror ("Large file name or no memory \n");
		return -1;
	}
	int out = open(first_file,O_WRONLY| O_CREAT | O_TRUNC , 0666);
	if (out == -1) {
		perror ("There is no such file \n");
		return -1;
	}
	char * second_file = NULL;
	second_file = get_fileaname();
	if (second_file == NULL) {
		perror ("Large file name or no memory\n");
		return -1;
	}
	int out2 = open(second_file,O_WRONLY| O_CREAT | O_TRUNC , 0666);
	if (out2 == -1) {
		perror ("There is no such file \n");
		return -1;
	}

    int fd_for_input = open("first_mmf.txt", O_RDWR | O_CREAT | O_TRUNC , 0777);
	ftruncate (fd_for_input , 500*sizeof(int));
	char *file_mmf =  mmap(NULL, 500*sizeof(int), PROT_WRITE | PROT_READ , MAP_SHARED ,fd_for_input,0);

	int fd_for_input2 = open("second_mmf.txt", O_RDWR | O_CREAT | O_TRUNC , 0777);
	ftruncate (fd_for_input2 , 500*sizeof(int)); // Изменяет длинну файла на нужную
	char *file_mmf2 =  mmap(NULL, 500*sizeof(int), PROT_WRITE | PROT_READ , MAP_SHARED ,fd_for_input2,0);

	pid_t id = fork();
	if (id == 0){
        if (dup2(out, STDOUT_FILENO) == -1){ // fileno(stdout)
			perror ("dup2");
		}
		dup2(out,fileno(stdout));
        char str1[1];
        str1[0] = '1';
		execl("./child", "./child", str1, NULL);
		perror("execl");
        printf("lox");
	}
    else if (id < 0){
        perror ("fork\n");
		exit(0);
    }
	pid_t id2 = fork();
	if (id2 == 0){
		if (dup2(out2, STDOUT_FILENO) == -1){ // fileno(stdout)
			perror ("dup2");
		}
        char str2[1];
        str2[0] = '2';
		execl("./child", "./child", str2, NULL);
		perror("execl"); 
        printf("lox2");   
	}
    else if (id2 < 0){
        perror ("fork\n");
		exit(0);
    }	

	if (id > 0 ) {
		int c;
		int flag = 0;
        int k_1 = 0;
        int k_2 = 0;

		while  ((c = getchar()) != EOF) {
            if (k_1 == 500*sizeof(int)){
                k_1 = 0;
                flag ++;
            }
            else if (k_2 == 500*sizeof(int)){
                k_2 = 0;
                flag ++;
            }

			if (flag % 2 == 0){
				// write(fd[WRITE], &c, sizeof(int));			
                file_mmf[k_1] = (char)c;
                k_1 ++;
			} else {
				file_mmf2[k_2] = (char)c;
                k_2 ++;
			}

			if (c == '\n'){	
                if (flag % 2 == 0){
                    msync(file_mmf, 500*sizeof(int), MS_SYNC| MS_INVALIDATE);
			kill(id, SIGUSR1);
                }
                else{
                    msync(file_mmf2, 500*sizeof(int), MS_SYNC| MS_INVALIDATE);
			kill(id2, SIGUSR1);
                }
				flag ++;
			}

		}
        kill(id, SIGUSR2);
        kill(id2, SIGUSR2);
		remove ("first_mmf.txt");
		remove ("second_mmf.txt");
		// wait(NULL);
	}
}
