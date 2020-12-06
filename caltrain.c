#include <pthread.h>
#include "caltrain.h"

void station_init(struct station *station) {
    station->Fseats = 0;
    station->passengers_W = 0;
    station->passengers_L= 0;
    pthread_mutex_init(&(station->mutex), NULL);
    pthread_cond_init(&(station->Fseats_available), NULL);
    pthread_cond_init(&(station->passengers_OB), NULL);
}

void station_load_train(struct station *station, int count) {
    pthread_mutex_lock(&(station->mutex));
    if (!count || !station->passengers_W) {
        pthread_mutex_unlock(&(station->mutex)); 
        return;
    }
    station->Fseats = count;
    pthread_cond_broadcast(&(station->Fseats_available)); 
    pthread_cond_wait(&(station->passengers_OB), &(station->mutex));
    station->Fseats = 0;
    pthread_mutex_unlock(&(station->mutex)); 
}

void station_wait_for_train(struct station *station) {
    pthread_mutex_lock(&(station->mutex));
    station->passengers_W++;
    while (!station->Fseats)
        pthread_cond_wait(&(station->Fseats_available), &(station->mutex)); 
    station->passengers_W--;
    station->passengers_L++;
    station->Fseats--;
    pthread_mutex_unlock(&(station->mutex)); 
}

void station_on_board(struct station *station) {
    pthread_mutex_lock(&(station->mutex)); 
    station->passengers_L--;
    if (!station->passengers_L && !(station->passengers_W && station->Fseats)) 
        pthread_cond_signal(&(station->passengers_OB));
    pthread_mutex_unlock(&(station->mutex)); 
}