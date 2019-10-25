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
      printf("Error in sem_close on counter\n" );
      exit(-1);
    }
    res = sem_close(sem);
    if (res < 0){
      printf("Error in sem_close on sem\n" );
      exit(-1);
    }
    printf("everything was killed\n");
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
  printf("Hi! I am your friendly neighborhood reader, and I will read whatever the write sends me;\n Be careful though: make sure to make me read something before sending the quit message, or I will tell mom!\n");
  char* name;
  void* mem;
  if (argv[1]){
    printf("%s",argv[1]);
    name = argv[1];
    int fd;
    fd = shm_open(name, O_RDWR, 0666);
    if(fd < 0){
      printf("Cannot create shm, %s\n", strerror(errno));
      exit(-1);
    }
    int res = ftruncate(fd, SIZE_TOPIC);
    if(res < 0){
      printf("Cannot truncate shm, %s\n", strerror(errno));
      exit(-1);
    }
    void*topic =SharedCreate(CHANNEL, SIZE,1);
    mem = mmap(topic+SIZE, SIZE_TOPIC, PROT_READ, MAP_SHARED, fd, 0);
  }
  else {
    name = CHANNEL;
    mem = SharedCreate(name,SIZE,1);
  }
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
    printf("Error in sem_post on counter");
    exit(-1);
  }
  while(1){

    res = sem_wait(sem);

    if(res < 0){
      printf("Error in sem_wait on sem");
      exit(-1);
    }
    int offset=SharedRead(mem);
    printf ("ho letto\n");
    if(offset==-1) break;
    mem+=offset;
    sleep(1);
  }
  res = sem_wait(counter);
  if(res < 0){
    printf("Error in sem_wait on counter");
    exit(-1);
  }
  res = sem_close(counter);
  if (res < 0){
    printf("Error in sem_close on counter\n" );
    exit(-1);
  }
  res = sem_close(sem);
  if (res < 0){
    printf("Error in sem_close on sem\n" );
    exit(-1);
  }

  return 0;
}
