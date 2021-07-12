#include "stems.h"
#include "request.h"

//
// To run:
// 1. Edit config-ws.txt with following contents
//    <port number>
// 2. Run by typing executable file
//    ./server
// Most of the work is done within routines written in request.c
//

#define MAX_THREADPOOL 20
#define MAX_QUEUE 50
#define MAX 100

sem_t empty;
sem_t full;
sem_t lock;

struct requestQueue
{
  int conn;
  long arrivalTime;
};

int in = -1;
int out = -1;
struct requestQueue rQ[MAX];

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

void getargs_ws(int *port, int *poolNum, int *queueNum)
{
  FILE *fp;

  if ((fp = fopen("config-ws.txt", "r")) == NULL)
    unix_error("config-ws.txt file does not open.");

  fscanf(fp, "%d", port);
  fscanf(fp, "%d", poolNum);
  fscanf(fp, "%d", queueNum);
  fclose(fp);
}

void consumer(int queueNum)
{
  int connfd;
  long arrivalTime;

  while (1)
  {
    sem_wait(&full); //
    sem_wait(&lock);
    out++;
    out %= MAX;
    connfd = rQ[out].conn;
    arrivalTime = rQ[out].arrivalTime;
    sem_post(&lock);
    sem_post(&empty);
    requestHandle(connfd, arrivalTime);
    Close(connfd);
  }
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
  if (pthread_create(thread, attr, start_routine, arg) != 0)
  {
    perror("thread create error : ");
    exit(1);
  }
  return 0;
}

int main(void) //producer
{
  int listenfd, connfd, port, poolNum, queueNum, clientlen;
  struct sockaddr_in clientaddr;
  pthread_t p_thread[MAX_THREADPOOL];

  initWatch();
  getargs_ws(&port, &poolNum, &queueNum);

  if (poolNum > MAX_THREADPOOL)
  {
    printf("too many thread.\n");
    exit(1);
  }
  if (queueNum > MAX_QUEUE)
  {
    printf("too many queue.\n");
    exit(1);
  }

  listenfd = Open_listenfd(port);

  sem_init(&empty, 0, queueNum);
  sem_init(&full, 0, 0);
  sem_init(&lock, 0, 1);

  for (int i = 0; i < poolNum; i++)
  {
    Pthread_create(&p_thread[i], NULL, consumer, NULL);
  }

  while (1)
  {
    //producer
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
    //consumer(connfd);
    sem_wait(&empty); //
    sem_wait(&lock);
    in++;
    in %= MAX;
    rQ[in].conn = connfd;
    rQ[in].arrivalTime = getWatch();
    sem_post(&lock);
    sem_post(&full);
  }
  sem_destroy(&lock);
  sem_destroy(&full);
  sem_destroy(&empty);
  return (0);
}
