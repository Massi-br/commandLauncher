#ifndef CUSTOMER__H
#define CUSTOMER__H

#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "file_synch.h"

#define ARG_HELP       "help"
#define ARG__OPT_LONG     "--"
#define BUF_SIZE 1024
#define ERR_MSG_SIZE 2


#define OPEN_PIPE(pipe, fd, flags)                      \
    if (((pipe) = open((fd),(flags))) == -1) {          \
        perror("open");                                 \
        clean_client();                                 \
        exit(EXIT_FAILURE);                             \
    }


#define UNLINK_PIPE(pipe)       \
    if (unlink(pipe) == -1) {   \
        perror("unlink");       \
        exit(EXIT_FAILURE);     \
    }

#define CHECK_ERROR(ret, msg)   \
    if ((ret) == -1) {          \
        perror(msg);            \
        clean_client();         \
        exit(EXIT_FAILURE);     \
    }



// is_help_requested : teste si l'un des paramètres de la ligne de commande est
// égal à la concaténation des chaines désignées par les macroconstantes
// ARG__OPT_LONG et ARG_HELP
 bool is_help_requested(int argc, char *argv[]);

 int sig_catcher_client(void);

 int setpnames(commande *cmd);
 int mkpipes(commande *cmd);

void error_routine(int fd);
void write_routine(int fd);

// /*
//  * La fonction à exécuter par le thread
//  * qui va ecrire dans entrée standard de la commande
//  * Dans le cas d'une commande intéractive
//  */
// void *write_routine(void *args);

// /*
//  * La fonction à exécuter par le thread
//  * qui va lire la sortie d'erreur de la commande
//  * Dans le cas d'erreurs produits
//  */
// void *error_routine(void *args);


 void clean_client();

 

// 
 void checking(int argc, char *argv[]);

// usage : affiche l'aide et termine le programme en notifiant un succès à
// l'environnement d'exécution
 void usage(char *argv[]);


// 
 void handler_client(int signum);


#endif