# Command Launcher

Command Launcher est un programme qui agit en tant que serveur, exécutant des commandes fournies par les clients via une file synchronisée. Il utilise le modèle producteur-consommateur pour gérer les commandes de manière asynchrone.

## Fonctionnalités

- Utilise une file synchronisée pour gérer l'exécution des commandes de manière sécurisée.
- Communique avec les clients via des tubes nommés pour l'entrée, la sortie et les erreurs des commandes.

## Prérequis

- Système d'exploitation Linux
- GCC (GNU Compiler Collection) installé

## Installation

1. Cloner le dépôt :

   ```bash
   git clone https://github.com/Massi-br/commandLauncher.git
Bien sûr, voici une base pour un fichier README basé sur ce que j'ai compris de votre programme. Assurez-vous de personnaliser les sections en fonction des détails spécifiques de votre programme et des meilleures pratiques pour la documentation.

markdown

# Command Launcher

Command Launcher est un programme qui agit en tant que serveur, exécutant des commandes fournies par les clients via une file synchronisée. Il utilise le modèle producteur-consommateur pour gérer les commandes de manière asynchrone.

## Fonctionnalités

- Accepte des commandes fournies par les clients.
- Utilise une file synchronisée pour gérer l'exécution des commandes de manière sécurisée.
- Communique avec les clients via des tubes nommés pour l'entrée, la sortie et les erreurs des commandes.
- Permet la configuration des paramètres tels que le nombre maximal de paramètres d'une commande.

## Prérequis

- Système d'exploitation Linux
- GCC (GNU Compiler Collection) installé

## Installation

1. Cloner le dépôt :

   ```bash
   git clone https://github.com/Massi-br/commandLauncher.git

2. Se déplacer dans le répertoire du projet :
    cd commandLauncher

3. Compiler le programme :
    make

## Utilisation
1. Lancer le programme serveur :
    ./command_launcher

2. Envoyer des commandes depuis un client (exemple) :
    ./client my_command arg1 arg2