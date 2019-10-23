#include <SharedFunctions.h>

void* SharedCreate(char* name, int size, int flag){ //crea la shm e la mmappa per tutti i processi che ne hanno bisogno,viene chiamata UNA volta
  int fd;
  if (flag == 0){
  fd = shm_open(name, O_RDWR|O_CREAT, 0666);
}
else{
  fd = shm_open(name, O_RDWR, 0666);
}
  if(fd < 0){
    printf("Cannot create shm\n");
    exit(-1);
  }
  int res = ftruncate(fd, size);
  if(res < 0){
    printf("Cannot truncate shm\n");
    exit(-1);
  }
  void* memory;
  if (flag == 0){
   memory = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);

 }
 else{
   memory = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
 }
 return memory;
}

int SharedWrite(char* text, void* memory){ //scrive nella shm il messaggio, restituendo la lunghezza dello stesso, in modo che possa essere aggiunta al puntatore nel processo
  printf("%d\n",(int)strlen(text));
  char* buffer= (char*) memory;
  sprintf(buffer, "%s\n",text);
  printf("Sending the message...\n");
  int len= strlen(text)+1;
  return len;
}

int SharedRead(void* memory){ //legge il contenuto della shm
  char* buffer = (char*) memory;
  int i = 0;
  char* check = (char*) malloc (sizeof(char) * 60);
  while(buffer[i] != '\0'){
    printf("%c", buffer[i]);
    check[i] = buffer[i];
    i += 1;
  }
  if(!strcmp(check, "quit\n\n")){
    free(check);
    return -1;
  }
  free(check);
  return i;
}

int topicNum (topicList* topics){//calcola il numero di topics creati
  int i = 0;
  topicListItem* item = topics->head;
  while(item->next){
    i++;
    item=item->next;
  }
  return i;
}

void listTopic (topicList* topics){ //stampa una lista di tutti i topic momentaneamente esistenti in mem
  topicListItem* item = topics->head;
  topic* currentTopic;
  printf("----PRINTING TOPICS INFO----");
  while(item->next){
    currentTopic = item->item;
    printf("Topic name: %s, Topic size: %n KB, Topic occupied space: %n KB\n",currentTopic->name, &currentTopic->size, &currentTopic->msglength);
    item=item->next;
  }
  return;
}

topic* createTopic (char* name, int size, int flag, void* mem, topicList* topics){ //crea un topic contestualmente alla shared memory preallocata
  int n_topics=topicNum(topics);
  int fd;
  if (flag == 0){
    fd = shm_open(name, O_RDWR|O_CREAT, 0666);
  }
  else{
    fd = shm_open(name, O_RDWR, 0666);
  }
  if(fd < 0){
    printf("Cannot create shm\n");
    exit(-1);
  }
  int res = ftruncate(fd, size);
  if(res < 0){
    printf("Cannot truncate shm\n");
    exit(-1);
  }
  void* memory;
  if (flag == 0){
    memory = mmap(mem+sizeof(topicListItem)*n_topics, size, PROT_WRITE, MAP_SHARED, fd, 0);
  }
  else{
    memory = mmap(mem+sizeof(topicListItem)*n_topics, size, PROT_READ, MAP_SHARED, fd, 0);
  }
  topic* newtopic = (topic*) malloc (sizeof (topic));
  newtopic->name = name;
  newtopic->size = size;
  newtopic->msglength = 0;
  newtopic->memory = memory;
  topicListItem* newtopicitem;
  newtopicitem->item=newtopic;
  newtopicitem->next=NULL;
  newtopicitem->prec=topics->last;
  topics->last=newtopicitem;
  return newtopic;
}


void deleteTopic (char* name){ //distrugge un topic,TODO
  return;
}
