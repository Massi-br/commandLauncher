#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "luncher.h"
#include "log.h"
#include "file_synch.h"
// #include "customer.h"


void processCommands(sync_t *fifo, int fdlog) {
    quitter = false;

    while (quitter == false) {
        commande cmd;
        cmd = defiler(fifo);

        if (cmd.cmd_size == 0) {
            fprintf(stderr, "__defiler\n");
            break;
        }

        if (print_cmd_log(fdlog, cmd) == -1) {
            fprintf(stderr, "__print_cmd_log\n");
        }

        int errnum;
        pthread_attr_t attr;
        if ((errnum = pthread_attr_init(&attr)) != 0) {
            fprintf(stderr, "pthread_attr_init: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }

        if ((errnum = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
            fprintf(stderr, "pthread_attr_setdetachstate: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }

        pthread_t th_exec;
        if ((errnum = pthread_create(&th_exec, &attr, exec_routine, &cmd) != 0)) {
            fprintf(stderr, "pthread_create: %s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }
    }
}

int main(void) {
    /*
	 * Gestion de fichier log
	 */
	int fdlog;
    fdlog = openlog();
    if (fdlog == -1){
		exit(EXIT_FAILURE);
	}
	
	if (redirectlog(fdlog)) {
		exit(EXIT_FAILURE);
	}
	
	if (print_time_log(fdlog) == -1) {
		fprintf(stderr,"__print_time_log\n");
		exit(EXIT_FAILURE);
	}
	if (print_log(fdlog, "Démarrage du lanceur de commande\n") == -1) {
		fprintf(stderr,"__print_log\n");
		exit(EXIT_FAILURE);
	}
	
	/*
	 * Gestion des signaux d'interruption
	 * SIGINT, SIGQUIT, SIGTSTP, SIGHUP, SIGTERM
	 */

	if (sig_catcher_lanceur() == -1) {
		fprintf(stderr,"__sig_catcher_lanceur\n");
		exit(EXIT_FAILURE);
	}
	
	/*
	 * Gérer les zombis à travers un signal SIGCHILD
	 */
	if (gestionnaire_zombis() == -1) {
		fprintf(stderr,"__gestionnaire_zombis\n");
		exit(EXIT_FAILURE);
	}


    // Créer l'objet de la mémoire partagée
	int shm_fd;
	if ((shm_fd = openfifo(O_NEW_SHM)) == -1) {
		fprintf(stderr,"__openfifo\n");
		exit(EXIT_FAILURE);
	}
	
    // Obtention de la file synchronisée
    sync_t *fifo;
    if ((fifo = mapfifo(shm_fd)) == NULL) {
		fprintf(stderr,"__mapfifo\n");
		exit(EXIT_FAILURE);
	}
	
	//Initialiser la file synchronisée
	if (initfifo(fifo) == -1) {
		fprintf(stderr,"__initfifo\n");
		exit(EXIT_FAILURE);
	}

    processCommands(fifo, fdlog);


	//Détruire les sémaphores qui gèrent la file synchronisée
	if (detsem(fifo) == -1) {
		fprintf(stderr,"__detsem\n");
	}
	
	//Supprimer la file synchronisée
	if (suppfifo(shm_fd) == -1){
		fprintf(stderr,"__suppfifo\n");
	}
	
    if (print_time_log(fdlog) == -1) {
		fprintf(stderr,"__print_time_log\n");
		exit(EXIT_FAILURE);
	}
	if (print_log(fdlog, "Fermeture du lanceur\n") == -1) {
		fprintf(stderr,"__print_log\n");
		exit(EXIT_FAILURE);
	}
    //Fermer le fichier log
	if (closelog(fdlog) == -1) {
		fprintf(stderr,"__closelog\n");
	}
	//Sortir du thread principal
    pthread_exit(NULL);
}


void *exec_routine(void *args) {

    commande cmd = *((commande *)args);
    
    char *cmdline[cmd.cmd_size + 1];
    
    //Lire les args de la commande reçue
    for (int i = 0; i < cmd.cmd_size; i++) {

        cmdline[i] = cmd.cmd_txt[i];
    }
    
    cmdline[cmd.cmd_size] = NULL;
	printf("avant fork");
	//Se dupliquer pour effectuer un appel à exec
    switch (fork()) {
		case -1:
			perror("fork");
			pthread_exit(NULL);
			break;
		case 0:
			/*
			 * Rediriger les entrées/sorties du processus fils
			 * Vers le client
			 */
			if (redirect(cmd.pipe_in, O_RDONLY, STDIN_FILENO) == -1) {
				fprintf(stderr,"__redirect pipe_in\n");
				exit(EXIT_FAILURE);
			}
			fprintf(stdout, "pipe in \n ");

			if (redirect(cmd.pipe_err, O_WRONLY, STDERR_FILENO) ==-1) {
				fprintf(stderr,"__redirect pipe_err\n");
				exit(EXIT_FAILURE);
			}
			fprintf(stdout, "pipe err \n ");
			if (redirect(cmd.pipe_out, O_WRONLY, STDOUT_FILENO) ==-1) {
				fprintf(stderr,"__redirect pipe_out\n");
				exit(EXIT_FAILURE);
			}
			fprintf(stdout, "Redirection des pipes\n");
			
			execvp(cmdline[0], cmdline);
			perror("execvp");
			
			/*
			 * Ecrire dans le fichier log
			 * En  cas d'échec de traitement de la commande
			 */
			int fdlog;
			fdlog = openlog();
			
			if (fdlog == -1) {
				exit(EXIT_FAILURE);
			}
			
			//Rediriger la sortie d'erreur standard vers le fichier log
			redirectlog(fdlog);
			if (redirectlog(fdlog)) {
				exit(EXIT_FAILURE);
			}
			
			//Rédiger le message d'erreur
			if (print_time_log(fdlog) == -1){
				fprintf(stderr,"__print_time_log\n");
				exit(EXIT_FAILURE);
			}
			if (print_log(fdlog, "Echec de traitement de la commande ") == -1){
				fprintf(stderr,"__print_log\n");
			}
			for (int i = 0; i < cmd.cmd_size; i++) {
				if (print_log(fdlog, cmdline[i]) == -1) {
					fprintf(stderr,"__print_log\n");
					break;
				}
				if (print_log(fdlog, " ") == -1){
					fprintf(stderr,"__print_log\n");
					break;
				}
			}
			perror("execvp");
			
			//Fermer le fichier log
			if (closelog(fdlog) == -1){
				fprintf(stderr,"__closelog\n");
			}
			exit(EXIT_FAILURE);
			break;
		default:
			break;
    }
	
	//Terminer le thread appelant
    pthread_exit(NULL);
}
void handler_lanceur(int signum) {

	if (signum < 0) {
		fprintf(stderr, "Mauvais numéro de signal\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Lanceur interrompu par un signal %d\n", signum);
	quitter = true;
}

int gestionnaire_zombis() {
	
	struct sigaction action;
	int errno;
    action.sa_handler = SIG_DFL;
    action.sa_flags = SA_NOCLDWAIT;
    if (sigfillset(&action.sa_mask) == -1) {
        perror("sigfilltset");
        return -1;
    }
    if (sigaction(SIGCHLD, &action, NULL) == -1) {
        perror("sigfilltset");
        return -1;
    }
    return 0;
}

int redirect(char *pipeName, int flags, int newfd) {

	int fd = open(pipeName, flags);
	if (fd == -1) {
		perror("open");
		return -1;
	}
	
	if (dup2(fd, newfd) == -1) {
		perror("dup2");
		close(fd);
		return -1;
	}
	if (close(fd) == -1) {
		perror("close");
		return -1;
	}
	return 0;
}


int sig_catcher_lanceur(void) {

	struct sigaction action;
	action.sa_handler = handler_lanceur;
	action.sa_flags = 0;
	if (sigfillset(&action.sa_mask) == -1) {
		perror("sigfilltset");
		return -1;
	}
	if (sigaction(SIGINT, &action, NULL) == -1) {
		perror("sigaction SIGINT");
		return -1;
	}
	if (sigaction(SIGQUIT, &action, NULL) == -1) {
		perror("sigaction SIGQUIT");
		return -1;
	}
	if (sigaction(SIGTSTP, &action, NULL) == -1) {
		perror("sigaction SIGSTP");
		return -1;
	}
	if (sigaction(SIGHUP, &action, NULL) == -1) {
		perror("sigaction SIGHUP");
		return -1;
	}
	if (sigaction(SIGTERM, &action, NULL) == -1) {
		perror("sigaction SIGTERM");
		return -1;
	}

	return 0;
}