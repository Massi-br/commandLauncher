#include "file.h"

#define MAX_CMD 10
#define TUBE_NAME_SIZE 255
#define MAX_ARGS_NB 7
#define MAX_STRING_SIZE 255


typedef struct 
{
    char pipe_in[TUBE_NAME_SIZE];
    char pipe_out[TUBE_NAME_SIZE];
    char pipe_err[TUBE_NAME_SIZE];
    size_t cmd_size;
    char cmd[MAX_ARGS_NB][MAX_STRING_SIZE];

}command;

typedef struct 
{
    sem_t mutex;
    sem_t plein;
    sem_t vide;
    size_t head;
    size_t tail;
    command buffer[MAX_CMD];

}file_synch;


void file_synch_init(file_synch *f){
    if (sem_init(&f->mutex,1,1)==-1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);  
    }
    if (sem_init(&f->plein,1,0)==-1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);  
    }
    if (sem_init(&f->vide,1,MAX_CMD)==-1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);  
    }

    f->head=0;
    f->tail=0;
}



int enfiler(command c ,file_synch *f ){

    if (sem_wait(&f->vide) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
    if (sem_wait(&f->mutex) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }

    f->buffer[f->head] = c;
    f->head = (f->head + 1) % MAX_CMD;

    if (sem_post(&f->mutex) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    if (sem_post(&f->plein) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}


command defiler(file_synch *f){
    command c ;
    if (sem_wait(&f->plein) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
    if (sem_wait(&f->mutex) == -1) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }

    c = f->buffer[f->tail];
    f->tail = (f->tail + 1) % MAX_CMD;

    if (sem_post(&f->mutex) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
    if (sem_post(&f->vide) == -1) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }

    return c;

}


int detsem(file_synch *f) {
	if (sem_destroy(&f->mutex) == -1) {
		perror("sem_destroy");
		return -1;
	}
	if (sem_destroy(&f->plein) == -1) {
		perror("sem_destroy");
		return -1;
	}
	if (sem_destroy(&f->vide) == -1) {
		perror("sem_destroy");
		return -1;
	}
	return 0;
}

