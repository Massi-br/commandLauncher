#ifndef LUNCHER__H
#define LUNCHER__H

// pour terminer le lanceur proprement
static volatile sig_atomic_t quitter; 


extern void handler_lanceur(int signum);

extern void *exec_routine(void *args);

extern int sig_catcher_lanceur(void);

extern int redirect(char *pipeName, int flags, int newfd);

extern int gestionnaire_zombis();

#endif