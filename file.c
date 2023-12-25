#include "file.h"



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

file_synch * open_map_fifo(){
  int shm_fd = shm_open(NOM_SHM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("shm_open");
    return NULL;
  }
  if (ftruncate(shm_fd, TAILLE_SHM) == -1) {
     perror("ftruncate");
     return NULL;
  }  
  volatile file_synch * f = (file_synch *) mmap(NULL, TAILLE_SHM, PROT_READ | PROT_WRITE,
                           MAP_SHARED, shm_fd, 0);
  if (f == MAP_FAILED) {
    perror("mmap");
    return NULL;
  }
  if (shm_unlink(NOM_SHM) == -1) {
    perror("shm_unlink");
    return NULL;
  }
  if (close(shm_fd) == -1) {
    perror("close");
    return NULL;
  }
  return f;
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


int detsem(file_synch *f){
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

