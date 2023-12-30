#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "log.h"


int main(void) {

	/*
	 * Gestion des signaux
	 * On bloque tous les signaux
	 * Pour protéger le processus de se terminer
	 * Avant de lancer le lanceur
	 */
	sigset_t set, oldset;
	if (sigfillset(&set) == -1) {
		perror("sigfillset");
		exit(EXIT_FAILURE);
	}
	
	if (sigprocmask(SIG_SETMASK, &set, &oldset) == -1) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}
	
	/*
	 * Fermer les descripteurs standards
	 * Pour éviter de dérangement des entrées/sorties
	 */
	 int fdnull;
	if ((fdnull = open("/dev/null", O_RDONLY)) == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	if (dup2(fdnull, STDIN_FILENO) == -1) {
		perror("dup2");
		exit(EXIT_FAILURE);
	}

	if (dup2(fdnull, STDOUT_FILENO) == -1) {
		perror("dup2");
		exit(EXIT_FAILURE);
	}

	if (close(fdnull) == -1) {
		perror("dup2");
		exit(EXIT_FAILURE);
	}
	
	switch (fork()) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
			break;
		case 0:

			// On crée une nouvelle session
			if (setsid() == -1) {
				perror("setsid");
				exit(EXIT_FAILURE);
			}
			
			/*
			 * Gestion de fichier log
			 */
			int fdlog;
			fdlog = openlog();
			if (fdlog == -1){
				exit(EXIT_FAILURE);
			}
			//Rediriger la sortie d'erreur standard vers le fichier log
			if (redirectlog(fdlog) == -1) {
    			fprintf(stderr, "__redirectlog\n");
    			exit(EXIT_FAILURE);
			}
			
			if (print_time_log(fdlog) == -1) {
				fprintf(stderr,"__print_time_log\n");
				exit(EXIT_FAILURE);
			}
			if (print_log(fdlog, "Démarrage du lanceur de commande en mode daemon\n") == -1){
				fprintf(stderr,"__print_log\n");
				exit(EXIT_FAILURE);
			}
			
			/* Débloquer les signaux
			 * (Récupérer le mask par défaut)
			 */
			if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
				perror("sigprocmask");
				exit(EXIT_FAILURE);
			}
			
			execlp("./lanceur", "./lanceur", NULL);
			
			perror("execlp");
			
			if (print_log(fdlog, "Démarrage du lanceur en mode daemon echoué\n") == -1){
				fprintf(stderr,"__print_log\n");
				exit(EXIT_FAILURE);
			}
			//Fermer le fichier log
			if (closelog(fdlog) == -1){
				fprintf(stderr,"__closelog\n");
			}
			exit(EXIT_FAILURE);
			break;
		default:
			//Débloquer les signaux
			if (sigprocmask(SIG_SETMASK, &oldset, NULL) == -1) {
				perror("sigprocmask");
				exit(EXIT_FAILURE);
			}
			break;
	}
    exit(EXIT_SUCCESS);
}
