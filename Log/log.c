#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

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

#include "log.h"
#include "file_synch.h"
#include "threads.h"

int openlog(void) {
	int fdlog;
	fdlog = open(LOG_NAME, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (fdlog == -1) {
		perror("open");
		return -1;
	}
	return fdlog;
}

int redirectlog(int fdlog) {
	if (dup2(fdlog, STDERR_FILENO) == -1) {
		perror("dup2");
		return -1;
	}
	return 0;
}

int print_time_log(int fdlog) {
	char buffer[50];
	time_t rawtime;
	struct tm *timeinfo;
	if (time(&rawtime) == -1) {
		perror("time");
		return -1;
	}
	timeinfo = localtime(&rawtime);
	if (timeinfo == NULL) {
		perror("localtime");
		return -1;
	}
	strftime(buffer, 50, "[%b %d - %H:%M:%S]\t", timeinfo);
	dprintf(fdlog, "%s", buffer);
	return 0;
}

int print_log(int fdlog, const char *msg) {
	if (dprintf(fdlog, "%s", msg) < 0) {
		perror("dprintf");
		return -1;
	}
	return 0;
}

int print_cmd_log(int fdlog, commande cmd) {
	if (print_time_log(fdlog) == -1) {
		return -1;
	}

	if (dprintf(fdlog, "Traitement de la commande : ") < 0) {
		perror("dprintf");
		return -1;
	}

	for (int i = 0; i < cmd.cmd_size; i++) {
		if (dprintf(fdlog, "%s ", cmd.cmd_txt[i]) < 0) {
			perror("dprintf");
			return -1;
		}
	}

	if (dprintf(fdlog, "\n") < 0) {
		perror("dprintf");
		return -1;
	}

	return 0;
}

int closelog(int fdlog) {
	if (close(fdlog) == -1) {
		perror("closeLog");
		return -1;
	}
	return 0;
}
