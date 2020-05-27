/*
    Author: M.Marconi
    Date:   27th May 2020
    Status: DONE
        PingPong.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_VALUE 1000000

int pipe_fdA[2];
int pipe_fdB[2];
int globalCounter;

int main(int argc, char * argv[]){
    int res;
    if(pipe(pipe_fdA) == -1){
        perror("pipe()");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe_fdB) == -1){
        perror("pipe()");
        exit(EXIT_FAILURE);
    }
    switch(fork()){
        case -1:
            perror("fork()");
            exit(EXIT_FAILURE);
            break;
        case 0:
            close(pipe_fdA[1]);
            close(pipe_fdB[0]);
            while(globalCounter < MAX_VALUE){
                res = read(pipe_fdA[0], &globalCounter, sizeof(int));
                if(res == -1){
                    perror("read()");
                    exit(EXIT_FAILURE);
                }
                if(res == 0){
                    printf("EOF\n");
                    break;
                }
                globalCounter++;
                res = write(pipe_fdB[1], &globalCounter, sizeof(int));
                if(res == -1){
                    perror("write()");
                    exit(EXIT_FAILURE);
                }
            }
            printf("[CHILD] Fuori da while.\n");
            exit(EXIT_SUCCESS);
        default:
            ;
    }
    close(pipe_fdA[0]);
    close(pipe_fdB[1]);
    while(globalCounter < MAX_VALUE){
        res = write(pipe_fdA[1], &globalCounter, sizeof(int));
        if(res == -1){
            perror("write()");
            exit(EXIT_FAILURE);
        }
        res = read(pipe_fdB[0], &globalCounter, sizeof(int));
        if(res == -1){
            perror("read()");
            exit(EXIT_FAILURE);
        }
        if(res == 0){
            printf("EOF\n");
            break;
        }
    }
    printf("[PARENT] Fuori da while.\n");
    printf("Final globalCounter value: %d\n", globalCounter);
    printf("Bye!\n");
    return 0;
}
