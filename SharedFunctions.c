#include <SharedFunctions.h>

void* shared_create(char* name, int size, int flag){ //crea la shm e la mmappa per tutti i processi che ne hanno bisogno,viene chiamata UNA volta
  int fd;
  if (flag == 0){
    fd = shm_open(name, O_CREAT|O_RDWR, 0666);
  }
  else{
    fd = shm_open(name, O_RDWR, 0666);
  }
  if(fd < 0){
    printf("Cannot create shm, %s\n", strerror(errno));
    exit(-1);
  }
  int res = ftruncate(fd, size);
  if(res < 0){
    printf("Cannot truncate shm, %s\n", strerror(errno));
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

int shared_write(char* text, void* memory){ //scrive nella shm il messaggio, restituendo la lunghezza dello stesso, in modo che possa essere aggiunta al puntatore nel processo
  printf("Length of the message: %d\n",(int)strlen(text));
  char* buffer= (char*) memory;
  printf("Sending the message...\n");
  sprintf(buffer, "%s\n",text);
  printf("Done!\n\n");
  int len= strlen(text)+1;
  return len;
}

int shared_read(void* memory){ //legge il contenuto della shm
  char* buffer = (char*) memory;
  int i = 0;
  char* check = (char*) malloc (sizeof(char) * 60);
  if(buffer[0]!= '\0') printf("New message received: ");
  while(buffer[i] != '\0'){
    if(i>1 && buffer[i-1]=='\n' && buffer[i-2]=='\n') printf("New message received: ");
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

int topic_num (topicList* topics){ //calcola il numero di topics creati
  int i = 1;
  topicListItem* item = topics->head;
  if (item==NULL) return 0;
  if (item->item==NULL) return 0;
  while(item->next){
    i++;
    item=item->next;
  }
  return i;
}

topic* find_topic(char* name, topicList* list){ //cerca nella lista di topic se esiste già il topic di nome name e lo restituisce
  topicListItem* item = list->head;
  char* topicname;
  if (item==NULL) return NULL;
  while(item->next){
    topicname=item->item->name;
    if(!strcmp(topicname,name)) return item->item;
    item=item->next;
  }
  topicname=item->item->name;
  if(!strcmp(topicname,name)) return item->item;
  return NULL;
}

void list_topic (topicList* topics){ //stampa una lista di tutti i topic momentaneamente esistenti in mem
  topicListItem* item = topics->head;
  topic* currentTopic;
  printf("\n----PRINTING TOPICS INFO----\n");
  int n_topics= topic_num(topics);
  printf("Number of existing topics: %d\n", n_topics);
  if (n_topics==0) {
    printf("No topic currently available\n\n");
    return;
  }
  currentTopic =item->item;
  while(item->next){
    printf("Topic name: %s, Topic size: %d KB, Topic occupied space: %d KB\n",currentTopic->name, currentTopic->size, currentTopic->msglength);
    item=item->next;
    currentTopic=item->item;
  }
  printf("Topic name: %s, Topic size: %d KB, Topic occupied space: %d KB\n\n",currentTopic->name, currentTopic->size, currentTopic->msglength);
  return;
}

topic* create_topic (char name[60], int size, void* mem, topicList* topics){ //crea un topic contestualmente alla shared memory preallocata
  topic* foundTopic = find_topic(name,topics);
  if(foundTopic!=NULL) {
    printf("The topic %s already exists, connecting to it...\n", name);
    //foundTopic->memory+=foundTopic->msglength;
    printf("Done!\n");
    return foundTopic;
  }
  int fd;
  fd = shm_open(name, O_RDWR|O_CREAT, 0666);
  if(fd < 0){
    printf("Cannot create shm, %s\n", strerror(errno));
    exit(-1);
  }
  int res = ftruncate(fd, size);
  if(res < 0){
    printf("Cannot truncate shm, %s\n", strerror(errno));
    exit(-1);
  }
  void* memory;
  topic* newtopic = (topic*) malloc (sizeof (topic));
  memory = mmap(mem, size, PROT_WRITE, MAP_SHARED, fd, 0);
  strcpy(newtopic->name, name);
  newtopic->size = size;
  newtopic->msglength = 0;
  newtopic->memory = memory;
  topicListItem* newtopicitem=(topicListItem*) malloc(sizeof(topicListItem));
  newtopicitem->item=newtopic;
  newtopicitem->next=NULL;
  if(topics->head==NULL){
    newtopicitem->prec=NULL;
    topics->head=newtopicitem;
  }
  else{
    newtopicitem->prec=topics->last;
    topics->last->next=newtopicitem;
  }
  topics->last=newtopicitem;
  printf("\nThe topic %s has been created succesfully!\n", name);
  return newtopic;
}

void* attach_to_topic (char* name, int size, char* memName, int memSize){ //connette un reader ad un topic esistente
  int fd;
  fd = shm_open(name, O_RDWR, 0666);
  if(fd < 0){
    printf("Cannot create shm, %s\n", strerror(errno));
    exit(-1);
  }
  int res = ftruncate(fd, size);
  if(res < 0){
    printf("Cannot truncate shm, %s\n", strerror(errno));
    exit(-1);
  }
  void* topic = shared_create(memName, memSize, 1);
  void* mem = mmap(topic, size, PROT_READ, MAP_SHARED, fd, 0);
  return mem;
}

void delete_topic (topic* topic){ //distugge un topic
  shm_unlink(topic->memory);
  free(topic);
  return;
}

topicList* init_topic_list(){ //inizializza una topicList vuota
  topicList* list = (topicList*) malloc(sizeof(topicList));
  list->head=NULL;
  list->last=NULL;
  return list;
}

void destroy_topic_list(topicList* list) { //distrugge tutti i topic presenti in list, per poi distruggere la lista stessa
  topicListItem* item=list->head;
  if (item==NULL){
    free(list);
    printf("Topic list destroyed\n");
    return;
  }
  else if (item!=NULL && item->next==NULL){
    delete_topic(item->item);
    free(item);
    free(list);
    printf("Topic list destroyed\n");
    return;
  }
  while (item->next){
      topic* topic =item->item;
      delete_topic(topic);
      item=item->next;
      free(item->prec);
  }
  delete_topic(item->item);
  free(item);
  free(list);
  printf("Topic list destroyed\n");
  return;
}

void send_quit(topicList* list, char* string){ //manda su tutti i topic presenti in list il messaggio di quit
  topicListItem* item= list->head;
  void* memory;
  if(item==NULL) {
    printf("No topics to destroy\n");
    return;
  }
  while (item->next){
    printf("\nTopic %s, sending quit message\n", item->item->name);
    memory=item->item->memory;
    int no_use = shared_write(string, memory+item->item->msglength);
    item=item->next;
  }
  printf("\nTopic %s, sending quit message\n", item->item->name);
  memory=item->item->memory;
  int no_use = shared_write(string, memory+item->item->msglength);
  return;
}
