#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<semaphore.h>
#include <sys/time.h>
#define NUM 16

using namespace std;

struct thread{
	int start, end;
	int cur; //which thread
};

pthread_t Tid[NUM];
sem_t mutex[NUM];
sem_t level4;
sem_t complete;

int* arr;
int* arr_t;
struct thread multi[NUM];


void* partition(void *arg);
void* bubble(void *arg);
void* ST_sort(void *arg);
void ST_part(int start, int end,int level);
void ST_bubble(int start, int end);

int main(){
	struct timeval start, end;
	int sec, usec;
	string filename;
	int size;
	int err;

	cout << "please enter the input file name: ";
	cin >> filename;

	ifstream input(filename, ios::in);
	if(!input){
		cout << "fail to open " << filename << endl;
		return 0;
	}

	input >> size;

	arr = new int[size];
	arr_t= new int[size];
	
	for(int i=0;i<size;++i){ //initialize the array
		input >> arr[i];
		arr_t[i] = arr[i];
	}
	input.close();

	multi[1].start = 0;
	multi[1].end = size-1;
	
	//int err;
	for(int i=1;i<NUM;++i){ //create semaphore
		err = sem_init(&mutex[i], 0, 0); //int sem_init(sem_t *sem, int pshared, unsigned int value);
		if(err<0){
			cout << "fail  to  create mutex\n";
			exit(1); 
		}
	}
	sem_init(&complete, 0, 0);
	sem_init(&level4, 0, 0);

	// do the muti-threads sort
	gettimeofday(&start, 0);

	for(int i=1;i<NUM;++i){ 
		multi[i].cur = i;
		if(i<8)
			err = pthread_create(&Tid[i], NULL, partition, (void*)&multi[i]); //create threads for quick sort
		else
			err = pthread_create(&Tid[i], NULL, bubble, (void*)&multi[i]); //create threads for bubble sort
		
		if(err<0){
			cout << "fail to create thread\n";
			exit(1);
		}
	}

	sem_post(&mutex[1]);//signal the first thread
	sem_wait(&complete); //wait for the first thread to signal completion

	gettimeofday(&end, 0);
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec = start.tv_usec;

	cout <<"Mutli-thread elapsed time: " << sec + (usec/1000000.0) << " sec\n";
	
	ofstream output("output1.txt", ios::out);
	for(int i=0;i<size;++i)
		output << arr[i] <<" ";
	output.close();
	

	for(int i=0;i<NUM;++i){
		err = sem_destroy(&mutex[i]);
		if(err<0)
			cout << "fail to sem_destroy\n";
	}
	sem_destroy(&complete);
	sem_destroy(&level4);
	
	delete [] arr;

//////////////////////////////////////////////////////////////////////////////////////////

	gettimeofday(&start, 0);
	// do the single thread
	multi[0].start = 0;
	multi[0].end = size-1;
	multi[0].cur = 0;
	err = sem_init(&mutex[0], 0, 0);
	if(err<0){
		cout << "fail to mutex for ST\n";
		exit(1);
	}
	err = sem_init(&complete, 0, 0);
	
	err = pthread_create(&Tid[0], NULL, ST_sort, (void*)&multi[0]);
	if(err<0){
		cout << "fail to create thread for ST\n";
		exit(1);
	}
	sem_post(&mutex[0]);//signal ST thread
	sem_wait(&complete);

	gettimeofday(&end, 0);
	sec = end.tv_sec - start.tv_sec;
	usec = end.tv_usec = start.tv_usec;
	cout <<"Single thread elapsed time: " << sec + (usec/1000000.0) << " sec\n";
	
	output.open("output2.txt", ios::out);
	for(int i=0;i<size;++i)
		output << arr_t[i]<<" ";
	output.close();
	
	sem_destroy(&complete);
	sem_destroy(&mutex[0]);
	delete [] arr_t;
	
	pthread_exit(NULL);

	return 0;	
}

void* partition(void *arg){
	struct thread* tt = (struct thread*) arg;
	sem_wait(&mutex[tt->cur]); //wait on their own semaphore

	int i = tt->start;
	int j = tt->end;
	int pivot = arr[(tt->start + tt->end)/2];
	int tmp;

	while(i<=j){
		while(arr[i] < pivot)
			++i;
		while(arr[j] > pivot)
			--j;
		if(i<=j){
			tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
			++i;
			--j;
		}
	}

	int left = tt->cur*2;//tt->cur*2-1
	int right = tt->cur*2+1;//tt->cur*2

	multi[left].start = tt->start;
	multi[left].end = j;

	multi[right].start = i;
	multi[right].end = tt->end;

	sem_post(&mutex[left]);
	sem_post(&mutex[right]);

	if(tt->cur==1){
		//wait for the 4th level thread to signal
		int finish;
		while(1){
			sem_getvalue(&level4, &finish);
			if(finish==8){
				sem_post(&complete);
				return ((void*) 0);
			}
		}
	}
	else
		return ((void*) 0);
}

void* bubble(void *arg){
	struct thread* tt = (struct thread*) arg;
	sem_wait(&mutex[tt->cur]);

	int i, j;
	int start = tt->start;
	int end = tt->end;// or end = tt->end-1;
	int tmp;

	for(i=0;i<end-start;++i){// repeat N times
		for(j=start;j<end-i;++j){ // Last i elements are already in place
			if(arr[j] > arr[j+1]){
				tmp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = tmp;
			}
		}
	}

	sem_post(&level4);
	return ((void*) 0);
}

void* ST_sort(void *arg){
	struct thread* tt = (struct thread*) arg;
	sem_wait(&mutex[tt->cur]);
	
    int i = tt->start;
	int j = tt->end;
	int pivot = arr_t[(tt->start + tt->end)/2];
	int tmp;
 
    // partition
    while(i<=j){
		while(arr_t[i] < pivot)
			++i;
		while(arr_t[j] > pivot)
			--j;
		if(i<=j){
			tmp = arr_t[i];
			arr_t[i] = arr_t[j];
			arr_t[j] = tmp;
			++i;
			--j;
		}
	}

	ST_part(tt->start, j, 2);
	ST_part(i, tt->end, 2);
    
	sem_post(&complete);
	return ((void*)0);
}

void ST_part(int start, int end,int level){
	int i = start;
	int j = end;
	int pivot = arr_t[(start + end)/2];
	int tmp;
 
      // partition
    while(i<=j){
		while(arr_t[i] < pivot)
			++i;
		while(arr_t[j] > pivot)
			--j;
		if(i<=j){
			tmp = arr_t[i];
			arr_t[i] = arr_t[j];
			arr_t[j] = tmp;
			++i;
			--j;
		}
	}
	
	if(level!=3){
		ST_part(start, j, level+1);
		ST_part(i, end, level+1);
	}
	else{//level 3
		ST_bubble(start, j);
		ST_bubble(i, end);
	}
	
	return;
}

void ST_bubble(int start, int end){
	int i, j;
	int tmp;

	for(i=0;i<end-start;++i){// repeat N times
		for(j=start;j<end-i;++j){ // Last i elements are already in place
			if(arr_t[j] > arr_t[j+1]){
				tmp = arr_t[j];
				arr_t[j] = arr_t[j+1];
				arr_t[j+1] = tmp;
			}
		}
	}
	
	return;
}