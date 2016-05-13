#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	int fd1,fd2;

	int* ptr1;
	int* ptr2;
	int flag = PROT_WRITE | PROT_READ;
    int semVal;
	int userguessNum;

	sem_t* startSem = sem_open("startSem", 0);
    sem_t* clientNum = sem_open("clientNum", 0);
    sem_t* waitInputSem = sem_open("waitInputSem", 0);
    sem_t* waitOutputSem = sem_open("waitOutputSem", 0);
    sem_t* doOneClient = sem_open("doOneClient", 0);
    sem_t* nowStartGameSem = sem_open("nowStartGameSem", 0);
    sem_t* waitPrintSem = sem_open("waitPrintSem", 0);
    sem_t* notEnterSem = sem_open("notEnterSem", 0);

    if((fd1 = open("input.mm", O_RDWR, 0666)) < 0) {
	    perror("File open error");
	    exit(1);
	}

	if((ptr1 = (int *)mmap(0, 4096, flag, MAP_SHARED, fd1 ,0)) == NULL) {
	    perror("mmap error");
	    exit(1);
	}

	if((fd2 = open("output.mm", O_RDWR, 0666)) < 0) {
	    perror("File open error");
	    exit(1);
	}

	if((ptr2 = (int *)mmap(0, 4096, flag, MAP_SHARED, fd2 ,0)) == NULL) {
	    perror("mmap error");
	    exit(1);
	}

    sem_getvalue(notEnterSem, &semVal);
    if(semVal!=0) {
        printf("You can't enter this room!\n");
        return 0;
    }

    sem_getvalue(clientNum, &semVal);
    if(semVal==0) {
        printf("You can't enter this room!\n");
        return 0;
    }
	
    sem_wait(clientNum);
    sem_getvalue(startSem, &semVal);

	printf("======================\n");
	printf("     Hello %d\n", getpid());
	printf("     Game Start!!\n");
	printf("======================\n");

    sem_post(startSem);

    sem_wait(nowStartGameSem);

    while(1){
       
        sem_getvalue(doOneClient, &semVal);
        if(semVal >= 1){
            sem_wait(doOneClient);

            printf("input num(100~999) : ");
            scanf("%d", &userguessNum);
            ptr1[0] = userguessNum;
            ptr1[1] = getpid();

            sem_post(waitInputSem);

            sem_wait(waitOutputSem);
           
            sem_post(waitPrintSem);

            printf("[%d] %d : %d strike, %d ball\n", ptr1[1], ptr1[0], ptr2[0],
                ptr2[1]);
        
            if(ptr2[0]==3){
                printf("You Win :)\n");
                break;
            }

            sem_wait(doOneClient);


        }
        else{
            sem_wait(waitPrintSem);
            printf("[%d] %d : %d strike, %d ball\n", ptr1[1], ptr1[0], ptr2[0],
                    ptr2[1]);

            if(ptr2[0]==3){
                printf("You Lose :(\n");
                break;
            }

            sem_post(doOneClient);
            sem_post(doOneClient);

        }
    }
	
	close(fd1);
	close(fd2);
    sem_post(clientNum);
}
