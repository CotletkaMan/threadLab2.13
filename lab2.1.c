#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define _REENTRANT
#include <string.h>
#include <sys/time.h>
#include "another.h"

#define Res 1
#define Cond 1
#define dt 0.25
int x , y , total , numNodes;
double T, *result;

pthread_barrier_t firstStep , secondStep;

double Ifunction(double t){
	if(t < 5)
		return 10;
	return -10;
}

double Ufunction(double t){
	return 20;
}

int* fillSpan(int total , int totalNodes){
	int* span = (int*)malloc(sizeof(int) * total * 2);
	int i;
	for( i = 0 ; i < total ; i ++){
		span[2 * i] = totalNodes / total * i;
		span[2 * i + 1] =  span[2 * i] + totalNodes / total;
	}
	return span;
}

typedef struct{
	int id;
	int* localSpan;

} argThread;

void* action(void* arg){
	double t;
	int count;
	argThread* arg_t = (argThread*)arg;
  	for(t = 0 , count = 0; t < T ; t += dt , count++){
  		double* volt = &result[count * numNodes];
  		int iter;
  		for(iter = arg_t -> localSpan[0] ; iter < arg_t -> localSpan[1] ; iter++){
  			double left = 0 , right = 0 , up = 0 , down = 0;
  			if(iter / y != 0.) left = (volt[iter - y] - volt[iter]) / Res;
  			if(iter / y != x + 1) right = (volt[iter] - volt[iter + y]) / Res;
  			if(iter % y != 0.) down = (volt[iter - 1] - volt[iter]) / Res;
  			if(iter % y != y - 1) up = (volt[iter] - volt[iter + 1]) / Res;
  			volt[iter + numNodes] = volt[iter] + dt * (left + down - up - right) / Cond;
  		}
		
		pthread_barrier_wait(&firstStep);

  		if(!arg_t -> id){
  			volt[0 + numNodes] = Ufunction(t);
  			volt[2 * numNodes - 1] += Ifunction(t) * dt / Cond;
  		}
  		pthread_barrier_wait(&secondStep);
  	}
}

int main(int argc , char** argv){
	x = atoi(argv[1]);
	y = atoi(argv[2]);
	T = atof(argv[3]);
	total = atoi(argv[4]);

	numNodes = (x + 2) * y;

	struct timeval begin;
	struct timeval end;
	struct timezone zone;

	gettimeofday(&begin, &zone);

	result = (double*)malloc(sizeof(double) * numNodes * (int)(T / dt + 1));
	memset(result , 0. , numNodes * sizeof(double) * (int)(T / dt + 1));
	argThread* args = (argThread*)malloc(sizeof(argThread) * total);

	pthread_attr_t p_attr;
	pthread_t* ids = (pthread_t*)malloc((total - 1) * sizeof(pthread_t));
	pthread_attr_init(&p_attr);
	pthread_attr_setscope(&p_attr , PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&p_attr , PTHREAD_CREATE_JOINABLE);
	pthread_barrier_init(&firstStep , NULL , total);
	pthread_barrier_init(&secondStep , NULL , total);

  	double t;
	int* span = fillSpan(total , numNodes);
	int i;
	for(i = 0 ; i < total ; i++){
		args[i].id = i;
		args[i].localSpan = &span[i * 2];
		if(i)
			pthread_create(&ids[i] , &p_attr , action , (void*)(&args[i]));
	}
	action((void*)(args));

	gettimeofday(&end, &zone);
	fprintf(stderr , "Time executing :: %lu on number of process %d\n" , end.tv_sec * 1000000 + end.tv_usec - begin.tv_usec - begin.tv_sec * 1000000 , total);
	double max = writeToFile(result , numNodes * (int)(T / dt + 1) , numNodes , y , "report.txt");
	createScript(y , x + 2, (int)(T / dt + 1)  , max);

	free(result);	
	free(ids);
	free(span);
	pthread_attr_destroy(&p_attr);
	pthread_barrier_destroy(&firstStep);
	pthread_barrier_destroy(&secondStep);
	return 0;
}