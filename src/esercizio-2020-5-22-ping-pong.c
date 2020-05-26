#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#define MAX_VALUE 1000000

int globalCounter;
sem_t sem;

void process_1(int * pipe_fd_1[], int * pipe_fd_2[]){
    close(pipe_fd_1[0]); // Close read end of pipe_1
    close(pipe_fd_2[1]); // Close write end of pipe_2
    int * buffer_r = calloc(1, sizeof(int));
    int * buffer_w = calloc(1, sizeof(int));
    int loc;
    int res;
    while(globalCounter != MAX_VALUE){
        if(sem_wait(&sem) == -1){
            perror("sem_wait()");
            exit(EXIT_FAILURE);
        }
        //FASE CRITICA
        res = read(pipe_fd_2[0], buffer_r, sizeof(int));
        if(res == -1){
            perror("read()");
            exit(EXIT_FAILURE);
        }
        if(*buffer_r == globalCounter){
            printf("correct\n");
        }
        loc = globalCounter;
        loc++;
        globalCounter = loc;
        *buffer_w = loc;
        res = write(pipe_fd_1[1], buffer_w, sizeof(int));
        if(res == -1){
            perror("write()");
            exit(EXIT_FAILURE);
        }
        //FASE CRITICA
        if(sem_post(&sem) == -1){
            perror("sem_post()");
            exit(EXIT_FAILURE);
        }
         printf("[PARENT]: globalCounter incremented to: %d\n", globalCounter);
    }
}
void process_2(int * pipe_fd_1[], int * pipe_fd_2[]){
    close(pipe_fd_1[1]); // Close write end of pipe_1
    close(pipe_fd_2[0]); // Close read end of pipe_2
    int loc;
    int * buffer_r = calloc(1, sizeof(int));
    int * buffer_w = calloc(1, sizeof(int));
    int res;
    while(globalCounter != MAX_VALUE){
        if(sem_wait(&sem) == -1){
            perror("sem_wait()");
            exit(EXIT_FAILURE);
        }
        //FASE CRITICA
        res = read(pipe_fd_1[0], buffer_r, sizeof(int));
        if(res == -1){
            perror("read()");
            exit(EXIT_FAILURE);
        }
        if(*buffer_r == globalCounter){
            printf("correct\n");
        }
        loc = globalCounter;
        loc++;
        globalCounter = loc;
        *buffer_w = loc;
        res = write(pipe_fd_2[1], buffer_w, sizeof(int));
        if(res == -1){
            perror("write()");
            exit(EXIT_FAILURE);
        }
        //FASE CRITICA
        if(sem_post(&sem) == -1){
            perror("sem_post()");
            exit(EXIT_FAILURE);
        }
        printf("[CHILD]: globalCounter incremented to: %d\n", globalCounter);
    }
}

int main(int argc, char*argv){
    int pipe_fd_1[2];
    int pipe_fd_2[2];
    int res;
    int s;
    if (pipe(pipe_fd_1) == -1) {
		perror("pipe()");
		exit(EXIT_FAILURE);
	}
    if (pipe(pipe_fd_2) == -1) {
		perror("pipe()");
		exit(EXIT_FAILURE);
	}
    if(s = sem_init(&sem, 0,1) == -1){
        perror("sem_init()");
        exit(EXIT_FAILURE);
    }
    switch(fork()){
        case -1: /* Error */
            perror("fork()");
            exit(EXIT_FAILURE);
            break;
        case 0: /* Child process */
            process_2(&pipe_fd_1, &pipe_fd_2);
            break;
        default: /* Parent process */
            process_1(&pipe_fd_1, &pipe_fd_2);
    }
    printf("final globalCounter = %d\n", globalCounter);
    if(s = sem_destroy(&sem) == -1){
        perror("sem_init()");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
