#include "SharedFunctions.h"
#define SIZE 1024
#define SIZE_TOPIC 128
#define SEM_NAME1 "/semaphore"
#define SEM_NAME2 "/counter"
#define CHANNEL "disney"

void my_handler(int signum){
  int res;
  if(signum == SIGINT){
    sem_t* sem = sem_open(SEM_NAME1, 0);
    if(sem == SEM_FAILED){
      printf("Error in sem_open: %d\n", errno);
      exit(-1);
    }
    sem_t* counter = sem_open(SEM_NAME2, 0);
    if(sem == SEM_FAILED){
      printf("Error in sem_open: %d\n", errno);
      exit(-1);
    }
    sem_wait(counter);
    res = sem_close(counter);
    if (res < 0){
      printf("Error in sem_close on counter\n");
      exit(-1);
    }
    res = sem_close(sem);
    if (res < 0){
      printf("Error in sem_close on sem\n");
      exit(-1);
    }
    printf("Everything was killed\n");
    exit(-1);
  }

}

int main(int argc, char** argv){
  struct sigaction sa;
  sa.sa_handler = my_handler;
  if(sigaction(SIGINT, &sa, NULL)<0){
    perror("sigaction");
    exit(-1);
  }
  printf("Hi! I am a reader, and I will read whatever the writer sends me.\nBe careful though: make sure to make me read something before sending the quit message!\n\n");
  char* name;
  void* mem;
  if (argv[1]){
    name = argv[1];
    mem = attachToTopic(name, SIZE_TOPIC, CHANNEL, SIZE);
  }
  else {
    printf("You must input the name of the topic you want to connect to!\n");
    exit(-1);
  }
  printf("You are connected to topic %s\n\n",name);
  sem_t* sem = sem_open(SEM_NAME1, 0);
  if(sem == SEM_FAILED){
    printf("Error in sem_open: %d\n", errno);
    exit(-1);
  }
  sem_t* counter = sem_open(SEM_NAME2, 0);
  if(sem == SEM_FAILED){
    printf("Error in sem_open: %d\n", errno);
    exit(-1);
  }
  int res = sem_post(counter);

  if(res < 0){
    printf("Error in sem_post on counter\n");
    exit(-1);
  }
  while(1){

    res = sem_wait(sem);

    if(res < 0){
      printf("Error in sem_wait on sem\n");
      exit(-1);
    }
    int offset=SharedRead(mem);
    if(offset==-1) break;
    mem+=offset;
    sleep(1);
  }
  res = sem_wait(counter);
  if(res < 0){
    printf("Error in sem_wait on counter\n");
    exit(-1);
  }
  res = sem_close(counter);
  if (res < 0){
    printf("Error in sem_close on counter\n");
    exit(-1);
  }
  res = sem_close(sem);
  if (res < 0){
    printf("Error in sem_close on sem\n");
    exit(-1);
  }

  return 0;
}
