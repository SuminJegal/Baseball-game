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
#include <time.h>

int r1, r2, r3;
int g1, g2, g3;
int strike = 0;
int ball = 0;
int strikecheck = 0;
int overOneStrike;
 
int createRandomNum() {
   	int rnum;
    srand(time(NULL));
	rnum = (rand() % 900) + 100;
    r1 = rnum/100;
    r2 = rnum/10 - (r1*10);
    r3 = rnum%10; 
    return rnum;
}

void breakNum(int gnum){
    g1 = gnum/100;
    g2 = gnum/10 - (g1*10);
    g3 = gnum%10;
}

int isStrike(){
   strike = 0;
   overOneStrike = 0;
   strikecheck = 0;
    if(r1 == g1){
       strike++;
       strikecheck = 1;
    }
   if(r2 == g2){
       strike++;
       strikecheck = 2;
    }
   if(r3 == g3){
       strike++;
       strikecheck = 3;
    }
   if(strike>=2){
        overOneStrike = 1;
    }
   return strike;
}

int isBall(){
    ball = 0;
    int ballcheck[3] = {0,};
    if(overOneStrike == 1) { 
        return 0;
    }
    else if(strike == 1) {
        switch(strikecheck){
            case 1:
                if(r2==g3) ball++;
                if(r3==g2) ball++;
                break;
            case 2:
                if(r1==g3) ball++;
                if(r3==g1) ball++;
                break;
            case 3:
                if(r1==g2) ball++;
                if(r2==g1) ball++;
                break;
        }
        return ball;
    }
    else{
        if(r1==g2){
            ball++;
            ballcheck[1] = 1;
        }
        else if(r1==g3){
            ball++;
            ballcheck[2] = 1;
        }
        if(r2==g1){
            ball++;
            ballcheck[0] = 1;
        }
        else if(r2==g3 && !ballcheck[2]){
            ball++;
        }
        if(r3==g1 && !ballcheck[0]){
            ball++;
        }
        else if(r3==g2 && !ballcheck[1]){
            ball++;
        }
        return ball;       
    }
}

int main(int argc, char** argv)
{
	int fd1,fd2;

	int* ptr1;
	int* ptr2;
	int semVal;
	int zero = 0;
    int ranNum;

	sem_t* startSem;
    sem_t* clientNum;
    sem_t* waitInputSem;
    sem_t* waitOutputSem;
    sem_t* doOneClient;
    sem_t* nowStartGameSem;
    sem_t* waitPrintSem;
    sem_t* notEnterSem;

	fd1 = open("input.mm", O_RDWR|O_CREAT, S_IRWXU);
	write(fd1, &zero, sizeof(int));
	ptr1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE , MAP_SHARED, fd1, 0);
	close(fd1);
	
	fd2 = open("output.mm", O_RDWR|O_CREAT, S_IRWXU);
	write(fd2, &zero, sizeof(int));
	ptr2 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
	close(fd2);

	sem_unlink("startSem");
	if((startSem = sem_open("startSem", O_CREAT, 0644, 0)) == SEM_FAILED) {
	    perror("open");
	    exit(1);
	}

    sem_unlink("clientNum");
    if((clientNum = sem_open("clientNum", O_CREAT, 0644, 2)) == SEM_FAILED) { 
        perror("open");
        exit(1);
     }

    sem_unlink("waitInputSem");
    if((waitInputSem = sem_open("waitInputSem", O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("open");
        exit(1);
    }

    sem_unlink("waitOutputSem");
    if((waitOutputSem = sem_open("waitOutputSem", O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("open");
        exit(1);
    }

    sem_unlink("doOneClient");
    if((doOneClient = sem_open("doOneClient", O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("open");
        exit(1);
    }
    
    sem_unlink("nowStartGameSem");
    if((nowStartGameSem = sem_open("nowStartGameSem", O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("open");
        exit(1);
    }

    sem_unlink("waitPrintSem");
    if((waitPrintSem = sem_open("waitPrintSem", O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("open");
        exit(1);
    }

    sem_unlink("notEnterSem");
    if((notEnterSem = sem_open("notEnterSem", O_CREAT, 0644, 0)) == SEM_FAILED) {
        perror("open");
        exit(1);
    }

	ranNum = createRandomNum();

	printf("rand num : %d\n",ranNum);
	
	sem_wait(startSem);
	printf("client hi\n");
	sem_wait(startSem);
	printf("client hi\n");

	printf("=========================\n");
	printf("        Game Start       \n");
	printf("=========================\n");

    sem_post(nowStartGameSem);
    sem_post(nowStartGameSem);

    while(ptr2[0]!=3){
        sem_wait(waitInputSem);

        breakNum(ptr1[0]);
        strike = isStrike();
        ball = isBall();
        ptr2[0] = strike;
        ptr2[1] = ball;

        printf("[%d] %d : %d strike, %d ball\n", ptr1[1], ptr1[0], ptr2[0],
                ptr2[1]);

        if(ptr2[0] == 3){
            printf("%d win!\n", ptr1[1]);
        }
        
        sem_post(waitOutputSem);
    }

    sem_post(notEnterSem);
    sem_close(startSem);
	exit(0);
}
