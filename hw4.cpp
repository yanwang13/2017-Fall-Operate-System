#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<pthread.h>
#include<semaphore.h>
#include<sys/time.h>
#include<queue>

using namespace std;

void* getTask(void *arg);

int multi[8];//store info of thread
pthread_t tid[8];
sem_t mutex_t[8];
sem_t complete; //whether current sort is done
sem_t new_task;
bool all_done;
int* arr;
int* ans;

class task{
	public:
		int start;
		int end;
		int level;
};

class joblist{
	public:
		queue<task*> taskq;
		int task_count;
		int sort_count;
		sem_t mutex;
		sem_t count;
		
		joblist(){
			queue<task*> taskq;
			sem_init(&mutex, 0, 1); //mutext protecting the joblist
			sem_init(&count, 0, 1);
			task_count = 0;
			sort_count = 0;
		}
		~joblist(){
			sem_destroy(&mutex);
			sem_destroy(&count);
		}

		task* takeTask(){
			task* tt = NULL;	
			
			if(all_done)
				return tt;
			//sem_wait(&new_task);
			sem_wait(&mutex);
			
			tt = taskq.front();
			taskq.pop();
			
			sem_post(&mutex);
			
			return tt;
		}

		void addTask(task* tt){

			
			sem_wait(&mutex);

			taskq.push(tt);
			sem_post(&new_task);

			sem_post(&mutex);
		}
		
		void run(task* tt){

			if(tt->level<3)
				partition(tt->start, tt->end, tt->level);
			
			else if(tt->level==3)
				bubble(tt->start,tt->end);
			
			sem_wait(&count);
			++task_count;
			if(task_count==15){ //threads has done all work for current sort
				sem_post(&complete);
				
				task_count = 0;//reset the task count
				++sort_count;
				if(sort_count==8)
					all_done==true;
			}
			sem_post(&count);
			
			return;
		}
		
		void partition(int start, int end, int level){
			int i = start;
			int j = end;
			int pivot = ans[(start + end)/2];
			int tmp;

			while(i<=j){
				while(ans[i] < pivot)
					++i;
				while(ans[j] > pivot)
					--j;
				if(i<=j){
					tmp = ans[i];
					ans[i] = ans[j];
					ans[j] = tmp;
					++i;
					--j;
				}
			}
			
			
			task* tt_left = new task();
			task* tt_right = new task();
			tt_left->level = tt_right->level= level +1;

			tt_left->start = start;
			tt_left->end = j;
			this->addTask(tt_left);

			tt_right->start = i;
			tt_right->end = end;
			this->addTask(tt_right);

			return;
		}
		
		void bubble(int start, int end){
			int i, j;
			int tmp;
			for(i=0;i<end-start;++i){// repeat N times
				for(j=start;j<end-i;++j){ // Last i elements are already in place
					if(ans[j] > ans[j+1]){
						tmp = ans[j];
						ans[j] = ans[j+1];
						ans[j+1] = tmp;
					}
				}
			}
			return;
		}
};

joblist jobs; //the queue which store the tasks

int main(){
	struct timeval start, end;
	int sec, usec;
	int size;
	int err;
	ofstream output;
	ifstream input("input.txt", ios::in);
	if(!input){
		cout << "fail to open input.txt" << endl;
		return 0;
	}

	input >> size;

	arr = new int[size];
	ans = new int[size];
	int tmp;
	for(int i=0;i<size;++i) //initialize the array
		input >> arr[i];
	input.close();

	all_done = false;
	
	sem_init(&new_task, 0, 0); //signals when new task arrive
	
	for(int i=0;i<8;++i)
		sem_init(&mutex_t[i], 0, 0);
	
	for(int i=0;i<8;++i){
		multi[i] = i;
		err = pthread_create(&tid[i], NULL, getTask, (void*)&multi[i]);
		if(err<0){
			cout << "fail to create thread\n";
			exit(1);
		}
	}

	for(int i=0;i<8;++i){

		for(int k=0;k<size;++k) //initialize the array
			ans[k] = arr[k];
		sem_init(&complete, 0, 0);
		
/////////////////////////////////////////////////////////
		gettimeofday(&start, 0);
		sem_post(&mutex_t[i]); //signal a new thread to do work
		
		task* tt = new task();//first task
		tt->start = 0;
		tt->end = size-1;
		tt->level = 0;

		jobs.addTask(tt);
		sem_wait(&complete);
		gettimeofday(&end, 0);
////////////////////////////////////////////////////////	
		
		sec = end.tv_sec - start.tv_sec;
		usec = end.tv_usec = start.tv_usec;
		cout <<"using " << i+1 << " threads --> elapsed time: "<< sec + (usec/1000000.0) << " sec\n";
		
		switch(i){
			case 0:
				output.open("output_1.txt", ios::out);
				break;
			case 1:
				output.open("output_2.txt", ios::out);
				break;
			case 2:
				output.open("output_3.txt", ios::out);
				break;
			case 3:
				output.open("output_4.txt", ios::out);
				break;
			case 4:
				output.open("output_5.txt", ios::out);
				break;
			case 5:
				output.open("output_6.txt", ios::out);
				break;
			case 6:
				output.open("output_7.txt", ios::out);
				break;
			case 7:
				output.open("output_8.txt", ios::out);
				break;
			
			default:
				break;
		}
		
		for(int k=0;k<size;++k)
			output << ans[k] << " ";
		output.close();

		sem_destroy(&complete);
		
	}

	sem_destroy(&new_task);
	delete [] arr;
	delete [] ans;
	return 0;	
}

void* getTask(void *arg){
	int* cur = (int*) arg;
	
	sem_wait(&mutex_t[*cur]); //wait for dispatcher to signal

	task* tt = NULL;

	while(1){
		
		sem_wait(&new_task);

		if((tt = jobs.takeTask())!=NULL){
			jobs.run(tt);
			delete tt;
		}

		if(all_done==true) //leave when all 8 sort are done
			break;
	}
	return ((void*) 0);
}
