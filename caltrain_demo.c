#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "caltrain.h"




struct load_train_args{
	struct station *station ;
	int Fseats;	
};
volatile int load_train_returned = 0;
volatile int threads_completed=0;
void *passenger_thread(void *arg)
{
	struct station *station = (struct station *) arg;
	station_wait_for_train(station);
	__sync_add_and_fetch(&threads_completed,1);
	return NULL;
}

void *load_train_thread(void *args)
{
  struct load_train_args *lta=(struct load_train_args *) args;
  station_load_train(lta->station,lta->Fseats);
  load_train_returned=1;
  return NULL;
}

const char *alarm_error_str;
int alarm_timeout;

void _alarm(int s , const char *error_str){
      alarm_timeout=s;
      alarm_error_str=error_str;
      alarm(s);
}

void alarm_handler(int fo)
{
	fprintf(stderr,"Error!!! Khong the hoan thanh sau %ds, loi co the xay ra [%s]",alarm_timeout,alarm_error_str);
	exit(1);
}
#define MIN(a, b) ((a) < (b)) ? (a) : (b)
void main(){
	struct  station station;
	station_init(&station);
	srandom(getpid()^time(NULL));
	signal(SIGALRM,alarm_handler);

	_alarm(1,"khong return khi khong co khach cho");
	station_load_train(&station,0);
	station_load_train(&station,10);
	_alarm(0,NULL);

    const int SL_passengers=100;
    int passengers_left=SL_passengers;
    for(int i=0;i<SL_passengers;i++)
    {
    	pthread_t t;
    	int r = pthread_create(&t,NULL,passenger_thread,&station);
    	if(r != 0)
    	{
    		perror("pthread_create");
    		exit(1);
    	}
    }

    _alarm(2,"station_load_train() khong return khi khong con cho trong");
    station_load_train(&station,0);
    _alarm(0,NULL);

    int SL_passengers_broad = 0;
    const int max_Fseats=50;
    int pass=0;
    while(passengers_left>0){
    	_alarm(2,"Khach khong len tau");
    	int Fseats=random()%max_Fseats;
    	printf("Tau vao tram voi %d ghe trong\n",Fseats );
    	load_train_returned=0;
    	struct load_train_args args={&station, Fseats};
    	pthread_t lt_t;
    	int r = pthread_create(&lt_t,NULL,load_train_thread,&args);
    	if(r!=0){
    		perror("loi tao luong");
    		exit(1);
    	}
    	int threads_to_reap=MIN(passengers_left,Fseats);
    	int threads_reaped=0;
    	while(threads_reaped<threads_to_reap){
    		if(load_train_returned){
    			fprintf(stderr, "ERROR!!! station_load_train return som\n" );
    			exit(1);
    		}
    		if(threads_completed>0){
    			if((pass % 2)==0)
    				usleep(random()%2);
    			threads_reaped++;
    			station_on_board(&station);
    			__sync_sub_and_fetch(&threads_completed,1);
    		}
    	}
        for (int i = 0; i < 1000; i++) {
            if (i > 50 && load_train_returned)
                break;
            usleep(1000);
        }

        if (!load_train_returned) {
            fprintf(stderr, "Error: station_load_train  return that bai\n");
            exit(1);
        }

        while (threads_completed > 0) {
            threads_reaped++;
            __sync_sub_and_fetch(&threads_completed, 1);
        }

        passengers_left -= threads_reaped;
        SL_passengers_broad += threads_reaped;
        printf("Tau khoi hanh voi %d hanh khach moi %s\n",
               threads_reaped,
               (threads_to_reap != threads_reaped) ? " *****" : "");

        if (threads_to_reap != threads_reaped) {
            fprintf(stderr, "Error: Qua nhieu hanh khach tren tau!\n");
            exit(1);
        }

        pass++;

    }
     if (SL_passengers_broad == SL_passengers) {
        return;
    } else {
        // I don't think this is reachable, but just in case.
        fprintf(stderr, "Error: du kien co  %d tong so hanh khach tren toa, nhung co %d!\n",
                SL_passengers, SL_passengers_broad);

   }
}