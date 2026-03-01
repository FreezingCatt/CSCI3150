#include "rwlock.h"
#include <stdio.h>
#include <string.h>

/*
    This is the ONLY file you need to implement.
*/

void rwlock_init(rwlock_t *rwlock) {
    pthread_mutex_init(&rwlock->lock, NULL);
    pthread_cond_init(&rwlock->readers_cond, NULL);
    pthread_cond_init(&rwlock->writers_cond, NULL);
    rwlock->active_readers = 0;
    rwlock->active_writers = 0;
    rwlock->waiting_high_priority_readers = 0;
    rwlock->waiting_high_priority_writers = 0;
    rwlock->waiting_low_priority_readers = 0;
    rwlock->waiting_low_priority_writers = 0;
}

void rwlock_acquire_read(rwlock_t *rwlock, const char *priority) {
    pthread_mutex_lock(&rwlock->lock);
    if (strcmp(priority, "high priority") == 0) {
        rwlock->waiting_high_priority_readers++;
        while (!can_high_reader_acquire(rwlock)) {
            pthread_cond_wait(&rwlock->readers_cond, &rwlock->lock);
        }
        rwlock->waiting_high_priority_readers--;
    } else {
        rwlock->waiting_low_priority_readers++;
        while (!can_low_reader_acquire(rwlock)) {
            pthread_cond_wait(&rwlock->readers_cond, &rwlock->lock);
        }
        rwlock->waiting_low_priority_readers--;
    }  
    rwlock->active_readers++;
    pthread_mutex_unlock(&rwlock->lock);
}

void rwlock_release_read(rwlock_t *rwlock) {
    pthread_mutex_lock(&rwlock->lock);
    rwlock->active_readers--;
    if (rwlock->active_readers == 0) {
        pthread_cond_broadcast(&rwlock->writers_cond);
    }
    pthread_cond_broadcast(&rwlock->readers_cond);
    pthread_mutex_unlock(&rwlock->lock);
}

void rwlock_acquire_write(rwlock_t *rwlock, const char *priority) {
    pthread_mutex_lock(&rwlock->lock);
    if (strcmp(priority, "high priority") == 0) {
        rwlock->waiting_high_priority_writers++;
        while (!can_high_writer_acquire(rwlock)) {
            pthread_cond_wait(&rwlock->writers_cond, &rwlock->lock);
        }
        rwlock->waiting_high_priority_writers--;
    } else {
        rwlock->waiting_low_priority_writers++;
        while (!can_low_writer_acquire(rwlock)) {
            pthread_cond_wait(&rwlock->writers_cond, &rwlock->lock);
        }
        rwlock->waiting_low_priority_writers--;
    }
    rwlock->active_writers = 1;
    pthread_mutex_unlock(&rwlock->lock);
}

void rwlock_release_write(rwlock_t *rwlock) {
    pthread_mutex_lock(&rwlock->lock);
    rwlock->active_writers = 0;
    pthread_cond_broadcast(&rwlock->writers_cond);
    pthread_cond_broadcast(&rwlock->readers_cond);
    pthread_mutex_unlock(&rwlock->lock);
}
