#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS_NB 7
#define MAX_CMD 10
#define TUBE_NAME_SIZE 255
#define MAX_STRING_SIZE 255
#define NOM_SHM "/MON_SHM12305"
#define TAILLE_SHM sizeof(file_synch)


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

void file_synch_init(file_synch *f);

file_synch * open_map_fifo();

int enfiler(command c ,file_synch *f );

command defiler(file_synch *f);

int detsem(file_synch *f);

file_synch * map_fifo();