#ifndef FILE_SYNCH__H
#define FILE_SYNCH__H

#include <semaphore.h>
#include <stdlib.h>


#define MAX_CMD 10
#define TAILLE_SHM (sizeof(sync_t))
#define MAX_ARGS_NB 7
#define MAX_STRING_SIZE 255
#define TUBE_NAME_SIZE 255

typedef struct {
	char pipe_in[TUBE_NAME_SIZE];
	char pipe_out[TUBE_NAME_SIZE];
	char pipe_err[TUBE_NAME_SIZE];
	int cmd_size;								
	char cmd_txt[MAX_ARGS_NB][MAX_STRING_SIZE];
} commande;

typedef struct {
	sem_t mutex;
	sem_t vide;
	sem_t plein;
	int tete;                	
	int queue;               	
	commande buffer[MAX_CMD];
} sync_t;

#define NOM_SHM "/synchronize_file_2024"

typedef enum {
    O_NEW_SHM ,
    O_EXISTED_SHM
}ShmOption;

#define GESTION_SEM(name, var)              \
if (sem_## name(&(var)) == -1) {            \
		perror("sem_error");                \
		return -1;                          \
	}

#define GESTION_SEM_CMD(name, var, cmd)             \
if (sem_## name(&(var)) == -1) {                    \
		perror("sem_error");                        \
        cmd.cmd_size = 0;                           \
		return cmd;                                 \
	}


/*
 * Ajouter une nouvelle demande dans la file par le client
 * Ou attendre si la file est pleine
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
extern int enfiler(commande cmd, sync_t *file);

/* 
 * Retirer une demande de la file par le lanceur
 * Ou attendre si la file est vide
 * Cette fonction retourne une demande vide en  cas d'echec
 */
extern commande defiler(sync_t *file);

/*
 * Définir les valeurs par default de la file synchronisée
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */ 
extern int initfifo(sync_t *file);

/*
 * Créer ou ouvrir un objet de mémoire partagée
 * Selon le int mode
 * O_NEW_SHM : pour Créer un nouveau objet mémoire partagée
 * (cas lanceur)
 * O_EXISTED_SHM : pour ouvrir un objet mémoire partagée déjà existant
 * (cas client)
 * Cette fonction retourne un un descripteur de fichier associé à l’objet en cas de succès
 * Ou -1 en cas d'echec
 */ 
extern int openfifo(int mode);

/*
 * Établir une projection en mémoire
 * Cette fonction  renvoie un pointeur sur la file synchronisée en cas de succès
 * Ou NULL en cas d'echec
 */
extern sync_t *mapfifo(int shm_fd);

/*
 * Supprimer la mémoire partagée
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
extern int suppfifo(int shm_fd);

/*
 * Detruire les sémaphores utilisés dans la file synchronisée
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
extern int detsem(sync_t *fifo);

#endif