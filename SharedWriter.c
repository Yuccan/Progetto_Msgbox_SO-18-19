#include "SharedFunctions.h"
#include <string.h>
#define SIZE 1024
#define SIZE_TOPIC 128
#define SEM_NAME1 "/semaphore"
#define SEM_NAME2 "/counter"
#define CHANNEL  "disney"

void my_handler(int signum){
  int res;
  if(signum == SIGINT){
    sem_t* sem = sem_open(SEM_NAME1, 0);
    if(sem == SEM_FAILED){
      printf("Error in sem_open: %s\n", strerror(errno));
      exit(-1);
    }
    sem_t* counter = sem_open(SEM_NAME2, 0);
    if(sem == SEM_FAILED){
      printf("Error in sem_open: %s\n", strerror(errno));
      exit(-1);
    }

    res = sem_close(counter);
    if (res < 0){
      printf("Error sem_close on counter\n");
      exit(-1);
    }
    res = sem_unlink(SEM_NAME2);
    if (res < 0){
      printf("Error sem_unlink on counter\n");
      exit(-1);
    }
    res = sem_close(sem);
    if (res < 0){
      printf("Error in sem_close on sem\n");
      exit(-1);
    }
    res = sem_unlink(SEM_NAME1);
    if (res < 0){
      printf("Error sem_unlink on sem\n");
      exit(-1);
    }
    shm_unlink(CHANNEL);
    printf("Everything was killed\n");
    exit(-1);
  }

}

int main(int argc, char** argv){
  printf("Hi, I am the writer! You can communicate with the readers through topics.\nPlease remember that the first input is the name of the topic!\nIf you want to close everything, use the 'quit' command.\n\n");

  struct sigaction sa;
  sa.sa_handler = my_handler;
  if(sigaction(SIGINT, &sa, NULL)<0){
    perror("sigaction\n");
    exit(-1);
  }
  int res;
  char* name= CHANNEL;
  void* mem= SharedCreate(name,SIZE,0);
  char* topicname= (char*) malloc (sizeof(char)*60);
  char* text= (char*) malloc (sizeof(char)*60);
  topicList* list= initTopicList();
  listTopic(list);
  topic* newtopic;
  void* topic;
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

  while(1){
    //lettura messaggio
    printf("\n-------------------\n---Outside topic---\n-------------------\n");
    printf("Please write the name of the topic you want to create (enter 'quit' to close):\n");
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
    //se la stringa è quit, faccio una sharedWrite a vuoto ed esco dal while(1)
    if(!strcmp(topicname,"quit\n")){
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
      sendQuit(list,topicname);
      free(num);
      break;
    }
    char strippedtopicname[60];
    strcpy(strippedtopicname,strtok(topicname,"\n"));
    //se sono qui la stringa non è quit, quindi creo il topic
    newtopic = createTopic(strippedtopicname, SIZE_TOPIC, mem, list);
    topic = newtopic->memory;
    topic += newtopic->msglength;
    listTopic(list);
    printf("\n------------------\n---Inside topic---\n------------------\n");
    printf("\nHello, you are now inside topic %s. If you want to change topic, please use the command 'exit'. And remember the 'quit' command is always available!\n\n", topicname);
    i = 0;
    //a questo punto entro nel while per la scrittura nel topic
    while(1){
      printf("Please write the message you want to send (enter 'quit' to close, 'exit' to change topic):\n");
      //leggo il messaggio
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
      //se il messaggio è quit, esco dal while (1), devo ancora vedere cosa succede fuori dal ciclo
      if(!strcmp(text,"quit\n")){
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
        sendQuit(list, text);
        free(num);
        break;
      }

      //se leggo exit, esco dal topic
      if (!strcmp(text, "exit\n")){
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
        newtopic->msglength+=no_use;
        printf("You are now outside of topic %s\n\n", topicname);
        free(num);
        break;
      }
      //ho letto il messaggio, lo scrivo nel topic

      int*number = (int*)malloc(sizeof(int));
      res = sem_getvalue(counter, number);

      if(res < 0){
        printf("Error in getvalue: %d\n", errno);
        exit(-1);
      }
      int z;
      printf("\nReader number:%d\n", *number);

      for(z = 0; z < *number; z++){
        res = sem_post(sem);
        if(res < 0){
          printf("Error in post number: %d\n", i+1);
          exit(-1);
        }
      }
      free(number);
      int offset=SharedWrite(text,topic);
      newtopic->msglength+=offset;
      topic+=offset;
      i=0;
      z=0;
      free(text);
      text= (char*) malloc (sizeof(char)*60);
    }
    //se il testo ricevuto era quit
    if (!strcmp(text, "quit\n")){
      free(text);
      break;
    }
    //altrimenti posso cambiare topic per cominciare a scrivere in quello
    i=0;
    free(text);
    text = (char*) malloc (sizeof(char)*60);
    free(topicname);
    topicname = (char*) malloc (sizeof(char)*60);
  }
  free(topicname);
  destroyTopicList(list);
  while(1){
    int*value = (int*)malloc(sizeof(int));
    res = sem_getvalue(counter, value);
    if(res < 0){
      printf("Error in getvalue: %s\n", strerror(errno));
      exit(-1);
    }
    if(*value == 0) {
      free(value);
      break;
    }
    free(value);
    sleep(1);
  }
  res = sem_close(counter);
  if (res < 0){
    printf("Error sem_close on counter\n");
    exit(-1);
  }
  res = sem_unlink(SEM_NAME2);
  if (res < 0){
    printf("Error sem_unlink on counter\n");
    exit(-1);
  }
  res = sem_close(sem);
  if (res < 0){
    printf("Error in sem_close on sem\n");
    exit(-1);
  }
  res = sem_unlink(SEM_NAME1);
  if (res < 0){
    printf("Error sem_unlink on sem\n");
    exit(-1);
  }
  shm_unlink(name);
  printf("Bye bye, call me again if you need me!\n");
  return 0;
}
