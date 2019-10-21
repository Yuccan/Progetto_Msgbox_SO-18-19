#include "SharedFunctions.h"
#include <string.h>
#define SIZE 1024
#define SIZE_TOPIC 128
#define SEM_NAME1 "/semaphore"
#define SEM_NAME2 "/counter"
#define CHANNEL  "disney"


//situazione corrente: il ciclo è circa completato, entro nel topic, bisogna sistemare cosa succede con exit e risolvere
//il problema che dà la sem_getvalue(blocca writer e dà errori sul reader)

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

    res = sem_close(counter);
    if (res < 0){
      printf("Error sem_close on counter\n" );
      exit(-1);
    }
    res = sem_unlink(SEM_NAME2);
    if (res < 0){
      printf("Error sem_unlink on counter\n" );
      exit(-1);
    }
    res = sem_close(sem);
    if (res < 0){
      printf("Error in sem_close on sem\n" );
      exit(-1);
    }
    res = sem_unlink(SEM_NAME1);
    if (res < 0){
      printf("Error sem_unlink on sem\n" );
      exit(-1);
    }
    shm_unlink(CHANNEL);
    printf("Ho ucciso tutto\n");
    exit(-1);
  }

}

int main(int argc, char** argv){
  printf("Hi! I am the Evil Writer!\n Those silly readers will make sure to read whatever I tell them, even closing when I say quit! MWAHAH, SO EVIL!\n");
  //beginning initialization
  struct sigaction sa;
  sa.sa_handler = my_handler;
  if(sigaction(SIGINT, &sa, NULL)<0){
    perror("sigaction");
    exit(-1);
  }
  int res;
  char*name= CHANNEL;
  void* mem= SharedCreate(name,SIZE,0);
  char* text= (char*) malloc (sizeof(char)*60);
  char* topicname= (char*) malloc (sizeof(char)*60);
  sem_t* sem = sem_open(SEM_NAME1, O_CREAT, 0666, 0);
  if(sem == SEM_FAILED){
    printf("Error in sem_open: %d\n", errno);
    exit(-1);
  }
  sem_t* counter = sem_open(SEM_NAME2, O_CREAT, 0666, 0);
  if(sem == SEM_FAILED){
    printf("Error in sem_open: %d\n", errno);
    exit(-1);
  }
  int i = 0;
  //initialization is over, onto the topic setting

  while(1){
    while(i< 60){
      res = read(0, topicname + i, 1);
      if(res == 1){
        if (topicname[i] == '\n') break;
        i++;
      }
      else{
        exit(-1);
      }
    }
    //topic name received, let's create it
    if(strcmp(topicname,"quit\n")){
      void* topic = createTopic(topicname, SIZE_TOPIC, 0, mem);
      printf("Hello, you are now inside topic %s \n", topicname);
      //topic created, now let's read our cute messages
      while(1){
        while(i< 60){
          res = read(0, text + i, 1);
          if(res == 1){
            if (text[i] == '\n') break;
            i++;
          }
          else{
            exit(-1);
          }
        }
        //message received! let's send it
        if(strcmp(text,"quit\n") && strcmp(text,"exit\n")){
          int*num = (int*)malloc(sizeof(int));
          res = sem_getvalue(counter, num);
          if(res < 0){
            printf("Error in getvalue: %d\n", errno);
            exit(-1);
          }
          int z;

          for(z = 0; z < *num; z++){
            res = sem_post(sem);
            if(res < 0){
              printf("Error in post number: %d\n", z+1);
              exit(-1);
            }
          }
          int no_use=SharedWrite(text,topic);
          break;
        }
        //message sent, let's release the readers

        int*number = (int*)malloc(sizeof(int));

        res = sem_getvalue(counter, number);

        if(res < 0){
          printf("Error in getvalue: %d\n", errno);
          exit(-1);
        }
        int z;
        printf("reader number:%d\n", *number);
        for(z = 0; z < *number; z++){
          res = sem_post(sem);

          if(res < 0){
            printf("Error in post number: %d\n", i+1);
            exit(-1);
          }
        }

        int offset=SharedWrite(text,topic);

        mem+=offset;
        i=0;
        z=0;
        free(text);
        text= (char*) malloc (sizeof(char)*60);
      }
      //quit message received inside topic
      free(text);
      while(1){
        int*value = (int*)malloc(sizeof(int));
        res = sem_getvalue(counter, value);
        if(res < 0){
          printf("Error in getvalue: %d\n", errno);
          exit(-1);
        }
        if(*value == 0) break;
        sleep(1);
      }
      res = sem_close(counter);
      if (res < 0){
        printf("Error sem_close on counter\n" );
        exit(-1);
      }
      res = sem_unlink(SEM_NAME2);
      if (res < 0){
        printf("Error sem_unlink on counter\n" );
        exit(-1);
      }
      res = sem_close(sem);
      if (res < 0){
        printf("Error in sem_close on sem\n" );
        exit(-1);
      }
      res = sem_unlink(SEM_NAME1);
      if (res < 0){
        printf("Error sem_unlink on sem\n" );
        exit(-1);
      }
      deleteTopic(topicname);
      shm_unlink(name);
      return 0;
    }
  }
  //outside of topic, must iterate

  //quit message received outside topic
  free(topicname);
  while(1){
    int*value = (int*)malloc(sizeof(int));
    res = sem_getvalue(counter, value);
    if(res < 0){
      printf("Error in getvalue: %d\n", errno);
      exit(-1);
    }
    if(*value == 0) break;
    sleep(1);
  }
  res = sem_close(counter);
  if (res < 0){
    printf("Error sem_close on counter\n" );
    exit(-1);
  }
  res = sem_unlink(SEM_NAME2);
  if (res < 0){
    printf("Error sem_unlink on counter\n" );
    exit(-1);
  }
  res = sem_close(sem);
  if (res < 0){
    printf("Error in sem_close on sem\n" );
    exit(-1);
  }
  res = sem_unlink(SEM_NAME1);
  if (res < 0){
    printf("Error sem_unlink on sem\n" );
    exit(-1);
  }
  shm_unlink(name);
  return 0;
}
