#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

struct station {
    int Fseats; 
    int passengers_W; 
    int passengers_L; 
    pthread_mutex_t mutex;
    pthread_cond_t passengers_OB; 
    pthread_cond_t Fseats_available; 
   
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);