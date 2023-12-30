#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>	
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "customer.h"
#include "file_synch.h"



int main(int argc, char *argv[]) {
    if (is_help_requested(argc, argv)) {
        usage(argv);
    }
    checking(argc, argv);

    /*
	 * Gestion des signaux d'interruption
	 * SIGINT, SIGQUIT, SIGTSTP, SIGHUP, SIGTERM
	 */
	if (sig_catcher_client() == -1) {
		fprintf(stderr,"sig_catcher_client\n");
		exit(EXIT_FAILURE);
	}

    // Créer les tubes nommés nécessaires pour effectuer une demande
	commande cmd;
	
	if (setpnames(&cmd) == -1) {
		fprintf(stderr,"setpnames\n");
		exit(EXIT_FAILURE);
	}
	

	if (mkpipes(&cmd) == -1) {
		fprintf(stderr,"mkpipes\n");
		exit(EXIT_FAILURE);
	}

	cmd.cmd_size = argc - 1;
	
	for (int i = 0; i < cmd.cmd_size; i++) {
		size_t size = strlen(argv[i + 1]) + 1;
		if (size > MAX_STRING_SIZE) {
			fprintf(stderr,"Taille de la commande depassée\n");
			clean_client();
			exit(EXIT_FAILURE);
		}
		strncpy(cmd.cmd_txt[i], argv[i + 1], sizeof(cmd.cmd_txt[i]) - 1);
		cmd.cmd_txt[i][sizeof(cmd.cmd_txt[i]) - 1] = '\0';
	}

    // Ouverture de l'objet de la mémoire partagée
	int shm_fd;
	if ((shm_fd = openfifo(O_EXISTED_SHM)) == -1) {
		fprintf(stderr,"__openfifo\n");
		exit(EXIT_FAILURE);
	}

    // Obtention de la file synchronisée
    sync_t *fifo;
    if ((fifo = mapfifo(shm_fd)) == NULL) {
		fprintf(stderr,"__mapfifo\n");
		exit(EXIT_FAILURE);
	}

    // Envoyer la demande dans la file synchronisée
	if (enfiler(cmd, fifo) == -1) {
		fprintf(stderr,"__enfiler\n");
		exit(EXIT_FAILURE);
	}

	
	int cmd_stdin;
    OPEN_PIPE(cmd_stdin, cmd.pipe_in, O_WRONLY);
    UNLINK_PIPE(cmd.pipe_in);

	/*
	 * Ecrire dans entrée standard de la commande à exécuter
	 * (Le cas d'une commande intéractive)
	 */
	// pthread_t th_write;
	// int errnum;
	// if ((errnum = pthread_create(&th_write, NULL, write_routine, &cmd_stdin) != 0)) {
	// 	fprintf(stderr, "pthread_create: %s\n", strerror(errnum));
	// 	clean_client();
	// 	exit(EXIT_FAILURE);
	// }

	write_routine(cmd_stdin);
    
	int cmd_stderr;
    OPEN_PIPE(cmd_stderr, cmd.pipe_err, O_RDONLY);
    UNLINK_PIPE(cmd.pipe_err);
	
	/*
	 * Lire la sortie d'erreurs de la commande
	 * En cas d'erreurs produits au moment de l'éxécution
	 */
	// pthread_t th_err;
	// if ((errnum = pthread_create(&th_err, NULL, error_routine, &cmd_stderr) != 0)) {
	// 	fprintf(stderr, "pthread_create: %s\n", strerror(errnum));
	// 	clean_client();
	// 	exit(EXIT_FAILURE);
	// }

	error_routine(cmd_stderr);



    int cmd_stdout;
    OPEN_PIPE(cmd_stdout, cmd.pipe_out, O_RDONLY);
    UNLINK_PIPE(cmd.pipe_out);


	char buffer[BUF_SIZE];
	ssize_t n;
	
	while ((n = read(cmd_stdout, buffer, sizeof(buffer))) > 0) {
		if (write(STDOUT_FILENO, buffer, (size_t)n) == -1) {
			perror("write");
			clean_client();
			exit(EXIT_FAILURE);
		}
	}
	
	CHECK_ERROR(n, "read")
	
	//Libérer les ressources occupés par le client
	clean_client();
    return EXIT_SUCCESS;
}

void handler_client(int signum) {
	if (signum < 0) {
		fprintf(stderr, "Mauvais numéro de signal\n");
	}
	fprintf(stderr, "\nClient interrompu par un signal %d\n", signum);
	clean_client();
	exit(EXIT_SUCCESS);
}

int setpnames(commande *cmd) {

    pid_t my_pid = getpid();

    if ((snprintf(cmd->pipe_in, TUBE_NAME_SIZE, "tube_in_%d", my_pid)) >= TUBE_NAME_SIZE) {
        fprintf(stderr, "tube_in : pas assez d'espace TUBE_NAME_SIZE (%d)\n", TUBE_NAME_SIZE);
        return -1;
    }
	

    if ((snprintf(cmd->pipe_out, TUBE_NAME_SIZE, "tube_out_%d", my_pid)) >= TUBE_NAME_SIZE) {
        fprintf(stderr, "tube_out : pas assez d'espace TUBE_NAME_SIZE (%d)\n", TUBE_NAME_SIZE);
        return -1;
    }

    if ((snprintf(cmd->pipe_err, TUBE_NAME_SIZE, "tube_err_%d", my_pid)) >= TUBE_NAME_SIZE) {
        fprintf(stderr, "tube_err : pas assez d'espace TUBE_NAME_SIZE (%d)\n", TUBE_NAME_SIZE);
        return -1;
    }

    return 0;
}


int mkpipes(commande *cmd) {

	if (mkfifo(cmd->pipe_in, S_IRUSR | S_IWUSR) == -1) {
		perror("mkfifo");
		return -1;
	}
	

	if (mkfifo(cmd->pipe_out, S_IRUSR | S_IWUSR) == -1) {
		perror("mkfifo");
		return -1;
	}

	if (mkfifo(cmd->pipe_err, S_IRUSR | S_IWUSR) == -1) {
		perror("mkfifo");
		return -1;
	}
	return 0;
}

int sig_catcher_client(void) {
	
    struct sigaction action;
    
    action.sa_handler = handler_client;
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
        perror("sigaction SIGTSTP");
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

void usage(char *argv[]) {
    printf("Usage : %s NAME_CMD [ARG_1 ... ARG_N]\n", argv[0]);
    printf("Le nombre d'arguments ne doit pas depasser %d\n", MAX_ARGS_NB);
    exit(EXIT_SUCCESS);
}

void checking(int argc, char *argv[]) {
    if (argc < 2 || argc > MAX_CMD + 2) {
        fprintf(stderr, "Saisissez « %s --help » pour plus d'informations.\n",
        argv[0]);
        exit(EXIT_FAILURE);
    }
    
}

bool is_help_requested(int argc, char *argv[]) {
  for (int k = 1; k < argc; ++k) {
    if (strcmp(argv[k], ARG__OPT_LONG ARG_HELP) == 0) {
      return true;
    }
  }
  return false;
}


void error_routine(int fd) {
	int errno;
	ssize_t n;
	char buffer[BUF_SIZE];

	while ((n = read(fd, buffer, BUF_SIZE)) > 0) {
		if (write(STDERR_FILENO, buffer, (size_t)n) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	if (n == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	if (close(fd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
}


void write_routine(int fd) {
	int errno;
	ssize_t n;
	char buffer[BUF_SIZE];

	while ((n = read(STDIN_FILENO, buffer, BUF_SIZE)) > 0) {
		if (write(fd, buffer, (size_t)n) == -1) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	if (n == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	if (close(fd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
}

void clean_client() {

    struct rlimit rlim;

    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) {
        perror("getrlimit");
        return;
    }

    for (long unsigned int i = 0; i <= rlim.rlim_max; ++i) {
		int fd = (int)i;
        close(fd);
    }
}















// void *write_routine(void *args) {
	
// 	int errnum;
// 	if ((errnum = pthread_detach(pthread_self())) != 0) {
// 		fprintf(stderr, "pthread_detach: %s\n", strerror(errnum));
// 		exit(EXIT_FAILURE);
// 	}
// 	int *fd = (int *)args;
// 	ssize_t n;
// 	char in[BUF_SIZE];

// 	while ((n = read(STDIN_FILENO, in, BUF_SIZE)) > 0) {
// 		if (write(*fd, in, (size_t)n) == -1) {
// 			perror("write");
// 			exit(EXIT_FAILURE);
// 		}
// 	}

// 	if (n == -1) {
// 		perror("read");
// 		exit(EXIT_FAILURE);
// 	}
// 	if (close(*fd) == -1) {
// 		perror("close");
// 		exit(EXIT_FAILURE);
// 	}
// 	pthread_exit(NULL);
// }

// void *error_routine(void *args) {
// 	int errnum;
// 	if ((errnum = pthread_detach(pthread_self())) != 0) {
// 		fprintf(stderr, "pthread_detach: %s\n", strerror(errnum));
// 		exit(EXIT_FAILURE);
// 	}
// 	int *fd = (int *)args;
// 	ssize_t n;
// 	char in[BUF_SIZE];

// 	while ((n = read(*fd, in, BUF_SIZE)) > 0) {
// 		if (write(STDERR_FILENO, in, (size_t)n) == -1) {
// 			perror("write");
// 			exit(EXIT_FAILURE);
// 		}
// 	}

// 	if (n == -1) {
// 		perror("read");
// 		exit(EXIT_FAILURE);
// 	}
// 	if (close(*fd) == -1) {
// 		perror("close");
// 		exit(EXIT_FAILURE);
// 	}
// 	pthread_exit(NULL);
// }
























