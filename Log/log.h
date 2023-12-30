#ifndef LOG_H
#define LOG_H

#define MAX_PATH 255
#define LOG_NAME "log"
#include "file_synch.h"

/*
 * Ouvrir un fichier log
 * Cette fonction retourne un descripteur de fichier log en cas de succès
 * Ou -1 en cas d'echec
 */
int openlog(void);

/*
 * Rediriger la sortie d'erreur standard
 * Vers le fichier log fdlog
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
int redirectlog(int fdlog);

/*
 * Ecrire la date et l'heure actuelles
 * dans le fichier log
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
int print_time_log(int fdlog);

/*
 * Ecrire le message msg dans le fichier log
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
int print_log(int fdlog, const char *msg);

/*
 * Enregistrer la commande reçu sur le fichier log
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
int print_cmd_log(int fdlog, commande cmd);

/*
 * Fermer le descripteur de fichier log
 * Cette fonction retourne 0 en cas de succès
 * Ou -1 en cas d'echec
 */
int closelog(int fdlog);

#endif
