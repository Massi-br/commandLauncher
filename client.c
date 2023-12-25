
#include "file.h"


int main(int argc , char *argv[]){
    if (argc < 2)
    {
        fprintf(stderr, "Nombre de paramètres insuffisant\n");
        fprintf(stderr, "usage : %s [cmd]  ", argv[0]);
        return EXIT_FAILURE; 
    }
    if(argc > MAX_ARGS_NB+1){
        fprintf(stderr, "le nombre maximum de paramètres est : %d",MAX_ARGS_NB);
        exit(EXIT_FAILURE);
    }

    command c;
    pid_t monPid= getpid();

    c.cmd_size = argc -1;

    if(sprintf(&c.pipe_in ,TUBE_NAME_SIZE,"pipe_in_%d",monPid) >= TUBE_NAME_SIZE){
        fprintf(stderr,"pipe_in : pas assez d'espace TUBE_NAME_SIZE (%d)\n", TUBE_NAME_SIZE);
        exit(EXIT_FAILURE);
    }
    if(sprintf(&c.pipe_out ,TUBE_NAME_SIZE,"pipe_in_%d",monPid) >= TUBE_NAME_SIZE){
        fprintf(stderr,"pipe_out : pas assez d'espace TUBE_NAME_SIZE (%d)\n", TUBE_NAME_SIZE);
        exit(EXIT_FAILURE);
    }
    if(sprintf(&c.pipe_err ,TUBE_NAME_SIZE,"pipe_in_%d",monPid) >= TUBE_NAME_SIZE){
        fprintf(stderr,"pipe_err : pas assez d'espace TUBE_NAME_SIZE (%d)\n", TUBE_NAME_SIZE);
        exit(EXIT_FAILURE);
    }


    if (mkfifo(c.pipe_in, S_IRUSR | S_IWUSR) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(c.pipe_out, S_IRUSR | S_IWUSR) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    if (mkfifo(c.pipe_err, S_IRUSR | S_IWUSR) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < c.cmd_size; i++)
    {
        if (strlen(argv[i+1]) > MAX_STRING_SIZE)
        {
            fprintf(stderr,"Taille de la commande dépasser. (Autorisé < (%d))\n",MAX_STRING_SIZE);
            exit(EXIT_FAILURE);
        }
        strncpy(c.cmd[i],argv[i+1], strlen(argv[i+1] + 1));
    }

    file_synch *f;
    file_synch_init(f);

    file_synch *fifo;
    fifo =open_map_fifo();
    if (fifo==NULL)
    {
        fprintf(stderr,"__mapfifo\n");
		exit(EXIT_FAILURE);
    }
    if (enfiler(c , fifo)==-1)
    {
        fprintf(stderr,"__enfiler\n");
		exit(EXIT_FAILURE);
    }

    // TODO: gestion d'écriture dans les tubes 

    






    return EXIT_SUCCESS;
}