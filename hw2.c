#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>

unsigned int* A;
unsigned int* C;
int shmid_a, shmid_c;


void init(int n);
void calculate(int n, int m);

int main(){
	
	struct timeval start, end;
	int input;
	unsigned int sum = 0;
	
	printf("Input the matrix dimension: ");
	scanf("%d", &input);
	printf("\n");
	
	init(input);
	
	int sec, usec;
	for(int i=1;i<=16;++i){
		gettimeofday(&start, 0);
		
		calculate(i, input); // forking process
		for(int i=0;i<input*input; ++i) sum += C[i]; //calculate sum

		gettimeofday(&end, 0);
		sec = end.tv_sec - start.tv_sec;
		usec = end.tv_usec = start.tv_usec;
		if(i==1)
			printf("Multipying matrices using 1 process\n");
		else
			printf("Multipying matrices using %d processes\n", i);
		printf("Elapsed time: %f sec, ", sec + (usec/1000000.0));
		printf("Checksum: %u\n", sum);
		sum = 0;
	}
	
	//detach memory
	if(shmdt(A)==-1){
		perror("shmdt a failed\n");	
		exit(1);
	}
	if(shmdt(C)==-1){
		perror("shmdt c failed\n");	
		exit(1);
	}
	
	if (shmctl(shmid_a, IPC_RMID, 0) == -1) {
		perror("shmctl a failed\n");
		exit(1);
	}
	if (shmctl(shmid_c, IPC_RMID, 0) == -1) {
		perror("shmctl c failed\n");
        exit(1);
	}
	return 0;
}

void init(int n){	//initila the matrix
	//create shared memory
	key_t key_a, key_c;
	size_t size = sizeof(unsigned int)*n*n;
	
	if((shmid_a = shmget(key_a, size, IPC_CREAT|0666))< 0){
		perror("shmget a failed\n");
		exit(1);
	}
	if((A = (unsigned int*)shmat(shmid_a, NULL, 0)) == (unsigned int*)-1) {
        perror("shmat a failed\n");
        exit(1);
    }

	for(int i=0;i<n;++i){
		for(int j=0;j<n;++j)
			A[i*n+j] = i*n+j;
	}
	if((shmid_c = shmget(key_c, size, IPC_CREAT|0666))< 0){
		perror("shmget c failed\n");
		exit(1);
	}
    if((C = (unsigned int*)shmat(shmid_c, NULL, 0)) == (unsigned int*)-1) {
        perror("shmat c failed\n");
        exit(1);
    }
	
}

// create n child process to do calculation
//martix is size m*m
void calculate(int n, int m){ 
	pid_t pid;
	int start = 0;
	int end = 0;
	int k = m/n;
	int rr = m%n;
	
	for(int i=0;i<n;++i){
		//end = (i==n-1)? start+k+(m%n) : start+k;
		end = (rr>0)? start+k+1 : start +k;
		--rr;

		if((pid=fork())<0){
			perror("fork failed\n");
			exit(1);
		}

		else if(pid==0){
			for(int i=start;i<end;++i){
				for(int j=0; j<m;++j){
					C[i*m+j] = 0;
					for(int k=0;k<m;++k)
						C[i*m+j] += A[i*m+k] * A[k*m+j];
				}
			}
			exit(0);
		}
		else 
			start = end;
	}
	
	while ((waitpid(-1, NULL, 0)) > 0);//wait for all child to finish
	
	return;
}
