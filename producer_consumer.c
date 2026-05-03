/*
 * CSE 3320 - Project 3 - Assignment 2
 *
 * Producer-consumer with a bounded circular buffer of size 5,
 * synchronized with one mutex and two condition variables.
 *   producer: reads chars from message.txt -> buffer
 *   consumer: reads chars from buffer       -> stdout
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int  in    = 0;            /* next slot to write */
int  out   = 0;            /* next slot to read  */
int  count = 0;            /* chars in buffer    */
int  producer_done = 0;    /* set when EOF hit   */

pthread_mutex_t mutex;
pthread_cond_t  not_full;
pthread_cond_t  not_empty;

void *producer(void *arg)
{
    FILE *fp = fopen("message.txt", "r");
    if (fp == NULL) {
        printf("ERROR: can't open message.txt!\n");
        pthread_mutex_lock(&mutex);
        producer_done = 1;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    int c;
    while ((c = fgetc(fp)) != EOF) {
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = (char)c;
        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    fclose(fp);

    pthread_mutex_lock(&mutex);
    producer_done = 1;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *consumer(void *arg)
{
    while (1) {
        pthread_mutex_lock(&mutex);

        while (count == 0 && !producer_done) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        if (count == 0 && producer_done) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        char c = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("%c", c);
        fflush(stdout);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
    }
    printf("\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t prod_tid, cons_tid;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full,  NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_create(&prod_tid, NULL, producer, NULL);
    pthread_create(&cons_tid, NULL, consumer, NULL);

    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
