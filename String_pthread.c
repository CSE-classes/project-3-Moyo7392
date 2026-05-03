#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_THREADS 4
#define MAX 1024

void *sub_string(void *);
int readf(FILE *fp);
int total=0;
int nlocal,n1,n2;
char *s1,*s2;
FILE *fp;
pthread_mutex_t total_lock;

int main(int argc, char *argv[])
{
    int i,rc;
    long t;
    pthread_t threads[NUM_THREADS];

    pthread_mutex_init(&total_lock,NULL);
    readf(fp);
    for(i=0;i<NUM_THREADS;i++){
        t = i;
        rc=pthread_create(&threads[i],NULL,sub_string,(void *)t);
        if (rc){
            printf("ERROR: return error from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for(i=0; i<NUM_THREADS; i++){
        rc = pthread_join(threads[i], NULL);
        if (rc){
            printf("ERROR: return error from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }
    printf("the occurences of s2 in s1 is %d\n",total);
    pthread_exit(0);
}

int readf(FILE *fp)
{
    if((fp=fopen("strings.txt", "r"))==NULL){
        printf("ERROR: can't open string.txt!\n");
        return 0;
    }
    s1=(char *)malloc(sizeof(char)*MAX);
    if(s1==NULL){
        printf("ERROR: Out of memory!\n");
        return -1;
    }
    s2=(char *)malloc(sizeof(char)*MAX);
    if(s1==NULL){
        printf("ERROR: Out of memory\n");
        return -1;
    }
    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1);
    n2=strlen(s2)-1;
    nlocal=n1/NUM_THREADS;
    if(s1==NULL || s2==NULL ||n1<n2)
        return -1;
    return 0;
}

/*
 * Each thread t handles starting positions [t*nlocal, (t+1)*nlocal).
 * The last thread caps at n1 - n2 + 1 so we don't run off the end.
 * We can safely read s1 across boundaries because n2 < nlocal,
 * which guarantees one thread's read never reaches the next-next thread.
 */
void *sub_string(void *threadid)
{
    long tid = (long)threadid;
    int start = tid * nlocal;
    int end   = start + nlocal;          /* exclusive */

    if (tid == NUM_THREADS - 1) {
        if (end > n1 - n2 + 1)
            end = n1 - n2 + 1;
    }

    int local = 0;
    int i, j, k, count;

    for (i = start; i < end; i++) {
        count = 0;
        for (j = i, k = 0; k < n2; j++, k++) {
            if (*(s1 + j) != *(s2 + k)) {
                break;
            } else {
                count++;
            }
            if (count == n2)
                local++;
        }
    }

    pthread_mutex_lock(&total_lock);
    total += local;
    pthread_mutex_unlock(&total_lock);

    pthread_exit(NULL);
}





