#pragma once
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

typedef struct topic {
  char name[60];
  int size;
  int msglength;
  void* memory;
} topic;

typedef struct topicListItem {
  topic* item;
  struct topicListItem* prec;
  struct topicListItem* next;
}topicListItem;

typedef struct topicList {
  topicListItem* head;
  topicListItem* last;
}topicList;

void* SharedCreate(char* name, int size, int flag); //crea la shm e la mmappa per tutti i processi che ne hanno bisogno, viene chiamata UNA volta AF MG FF
int SharedWrite(char* text, void* memory); //scrive nella shm il messaggio, restituendo la lunghezza dello stesso, in modo che possa essere aggiunta al puntatore nel processo AF MG FF
int SharedRead(void* memory); //legge il contenuto della shm AF MG FF
int topicNum (topicList* topics); //calcola il numero di topics creati
topic* findTopic(char* name, topicList* list); //cerca nella lista di topic se esiste già il topic di nome name e lo restituisce
void listTopic (topicList* topics); //stampa una lista di tutti i topic momentaneamente esistenti in mem
topic* createTopic (char* name, int size, void* mem, topicList* topics); //crea un topic contestualmente alla shared memory mem
void* attachToTopic (char* name, int size, char* memName, int memSize);
void deleteTopic (topic* topic); //distrugge un topic
topicList* initTopicList(); //inizializza una topicList vuota
void destroyTopicList(topicList* list); //distrugge tutti i topic presenti in list, per poi distruggere la lista stessa
void sendQuit(topicList* list, char* string); //manda su tutti i topic presenti in list il messaggio di quit
