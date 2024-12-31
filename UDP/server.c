#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "server.h"
#include "client.h"

static void init(void) {
    printf("✅ Initialisation du serveur...\n");
}

static void end(void) {
    printf("🛑 Fermeture du serveur...\n");
}

static void server_app(void) {
    //On va utiliser un tableau de structure Compte pour stocker plusieurs comptes
    Compte comptes[3] = {
      {"Ayoub7", "compte1", "password1", 213000, {{"AJOUT", "30/12/2024 17:50:17", 213000}}, 1},
      {"Ayoub26", "compte2", "password2", 100000, {{"AJOUT", "30/12/2024 17:52:21", 100000}}, 1},
      {"Ayoub93", "compte3", "password3", 50000, {{"AJOUT", "30/12/2024 17:54:34", 50000}}, 1}
    };

    SOCKET sock = server_init_connection();
    printf("🟢 Serveur UDP en écoute sur le port %d...\n", PORT);
    char buffer[BUF_SIZE];
    /* the index for the array */
    int actual = 0;
    int max = sock;
    /* an array for all clients */
    Client clients[MAX_CLIENTS];

    fd_set rdfs;

    while(1) {
        /*clear the set rdfs*/
        FD_ZERO(&rdfs);

        /* add STDIN_FILENO */
        FD_SET(STDIN_FILENO, &rdfs);

        /* add the connection socket */
        FD_SET(sock, &rdfs);

        if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1) {
            perror("select()");
            exit(errno);
        }

        /* something from standard input : i.e keyboard */
        if(FD_ISSET(STDIN_FILENO, &rdfs)) {
            /* stop process when type on keyboard */
            printf("🛑 Arrêt du serveur via le clavier.\n");
            break;
        }else if(FD_ISSET(sock, &rdfs)) {
            /* new client */
            SOCKADDR_IN csin = { 0 };
            /* a client is talking */
            read_client(sock, &csin, buffer);
            buffer[BUF_SIZE] = '\0';
            
            if(check_if_client_exists(clients, &csin, actual) == 0){
                if(actual != MAX_CLIENTS) {
                Client c = { csin };
                strncpy(c.name, buffer, BUF_SIZE - 1);
                clients[actual] = c;
                actual++;
                char welcome_message[BUF_SIZE];
                snprintf(welcome_message, BUF_SIZE,
                "🟢 Bienvenue %.50s sur le serveur bancaire UDP !\n"
                "Voici les commandes disponibles :\n"
                "- AJOUT <id_client> <id_compte> <password> <somme>\n"
                "- RETRAIT <id_client> <id_compte> <password> <somme>\n"
                "- SOLDE <id_client> <id_compte> <password>\n"
                "- OPERATIONS <id_client> <id_compte> <password>\n"
                "Veuillez respecter la syntaxe des commandes.\n",
                c.name);
                write_client(sock, &csin, welcome_message);
                printf("✅ Nouveau client ajouté : %s\n", c.name);
                }else {
                    printf("⚠️ Nombre maximal de clients atteint. Connexion refusée.\n");
                }
            }else {
                Client *client = get_client(clients, &csin, actual);
                if(client == NULL){
                    printf("❌ Erreur : client introuvable.\n");
                    continue;
                }
                traiter_commande(sock, &csin, comptes, 3, buffer);
                /*la ligne suivante permet de diffuser le message à tous les clients connectés*/
                /*on la commente pour ne pas diffuser les informations du compte aux autres clients*/
                //send_message_to_all_clients(sock, clients, client, actual, buffer, 0);
            }
        }
    }
    end_connection(sock);
}

//source : https://www.bien-programmer.fr/time.htm
void obtenir_date_heure(char *buffer, size_t buffer_size) {
    /* lire l'heure courante */
    time_t now = time(NULL);
    /* la convertir en heure locale */
    struct tm tm_now = *localtime(&now);
    /*Formate les informations de tm_now selon le format "JJ/MM/AAAA HH:MM:SS" et les stocke dans le buffer*/
    strftime(buffer, buffer_size, "%d/%m/%Y %H:%M:%S", &tm_now);
}

static void traiter_commande(SOCKET sock, SOCKADDR_IN *sin, Compte *comptes, int nb_comptes, const char *commande) {
    char requete[10], id_client[BUF_SIZE], id_compte[BUF_SIZE], password[BUF_SIZE];
    int somme;

    //Decoupage de la commande
    sscanf(commande, "%s %s %s %s %d", requete, id_client, id_compte, password, &somme);

    if (strcmp(requete, "AJOUT") == 0) {
        for (int i = 0; i < nb_comptes; i++) {
            //On verifie si les informations de la requete du client correpondent bien à un compte existant
            if (strcmp(comptes[i].id_client, id_client) == 0 && strcmp(comptes[i].id_compte, id_compte) == 0 && strcmp(comptes[i].password, password) == 0) {
                comptes[i].solde += somme;
                //Ajouter une nouvelle operation (AJOUT)
                if (comptes[i].nb_operations < 10) {
                    strcpy(comptes[i].operations[comptes[i].nb_operations].type_operation, "AJOUT");
                    obtenir_date_heure(comptes[i].operations[comptes[i].nb_operations].date_operation, 50);
                    comptes[i].operations[comptes[i].nb_operations].montant_operation = somme;
                    comptes[i].nb_operations++;
                }else {
                    //Decaler les operations si le tableau est plein
                    for (int j = 1; j < 10; j++) {
                        comptes[i].operations[j - 1] = comptes[i].operations[j];
                    }
                    strncpy(comptes[i].operations[9].type_operation, "AJOUT", 50);
                    obtenir_date_heure(comptes[i].operations[9].date_operation, 50);
                    comptes[i].operations[9].montant_operation = somme;
                }
                //Repondre au client
                write_client(sock, sin, "OK");
                printf("✅ AJOUT réussi : %d€ viennent d'être ajoutées au compte %s\n", somme, id_compte);
                return;
            }
        }
        //Si aucun compte ne correspond
        write_client(sock, sin, "KO\n");
        printf("❌ authentification incorrecte\n");
    }else if (strcmp(requete, "RETRAIT") == 0) {
        for (int i = 0; i < nb_comptes; i++) {
            //On verifie si les informations de la requete du client correpondent bien à un compte existant
            if (strcmp(comptes[i].id_client, id_client) == 0 && strcmp(comptes[i].id_compte, id_compte) == 0 && strcmp(comptes[i].password, password) == 0) {
                if (comptes[i].solde >= somme) {
                    comptes[i].solde -= somme;
                    //Ajouter une nouvelle operation (RETRAIT)
                    if (comptes[i].nb_operations < 10) {
                        strcpy(comptes[i].operations[comptes[i].nb_operations].type_operation, "RETRAIT");
                        obtenir_date_heure(comptes[i].operations[comptes[i].nb_operations].date_operation, 50);
                        comptes[i].operations[comptes[i].nb_operations].montant_operation = -somme;
                        comptes[i].nb_operations++;
                    }else {
                        //Decaler les operations si le tableau est plein
                        for (int j = 1; j < 10; j++) {
                            comptes[i].operations[j - 1] = comptes[i].operations[j];
                        }
                        strncpy(comptes[i].operations[9].type_operation, "RETRAIT", 50);
                        obtenir_date_heure(comptes[i].operations[9].date_operation, 50);
                        comptes[i].operations[9].montant_operation = -somme;
                    }
                    write_client(sock, sin, "OK\n");
                    printf("✅ RETRAIT réussi : %d€ viennent d'être retirées du compte %s\n", somme, id_compte);
                    return;
                } else {
                    write_client(sock, sin, "KO\n");
                    printf("❌ RETRAIT échoué : solde insuffisant\n");
                    printf("%s, votre solde est de %d€, vous ne pouvez pas retirer %d€\n",id_client, comptes[i].solde, somme);
                    return;
                }
            }
        }
        write_client(sock, sin, "KO\n");
        printf("❌ authentification incorrecte\n");
    }else if (strcmp(requete, "SOLDE") == 0) {
        for (int i = 0; i < nb_comptes; i++) {
            //On verifie si les informations de la requete du client correpondent bien a un compte existant
            if (strcmp(comptes[i].id_client, id_client) == 0 && strcmp(comptes[i].id_compte, id_compte) == 0 && strcmp(comptes[i].password, password) == 0) {
                //On verifie si le client a effectué des opérations
                char derniere_operation[50];
                if (comptes[i].nb_operations > 0) {
                    strcpy(derniere_operation, comptes[i].operations[comptes[i].nb_operations - 1].date_operation);
                }else {
                    strcpy(derniere_operation, "Aucune opération enregistrée dans ce compte");
                }
                //Répondre au client
                char reponse[BUF_SIZE];
                snprintf(reponse, BUF_SIZE, "RES_SOLDE Votre solde est de %d€. Dernière opération : %s\n", comptes[i].solde, derniere_operation);
                write_client(sock, sin, reponse);
                printf("✅ SOLDE : %s, votre solde est de %d€\n", id_client, comptes[i].solde);
                return;
            }
        }
        write_client(sock, sin, "KO\n");
        printf("❌ Authentification incorrecte\n");
    }else if (strcmp(requete, "OPERATIONS") == 0) {
    for (int i = 0; i < nb_comptes; i++) {
        if (strcmp(comptes[i].id_client, id_client) == 0 && strcmp(comptes[i].id_compte, id_compte) == 0 && strcmp(comptes[i].password, password) == 0) {
            char operations[BUF_SIZE] = "RES_OPERATIONS\n";
            for (int j = 0; j < comptes[i].nb_operations; j++) {
                char operation[128];
                snprintf(operation, sizeof(operation), "%s %s %d€\n", comptes[i].operations[j].type_operation, comptes[i].operations[j].date_operation, comptes[i].operations[j].montant_operation);
                strncat(operations, operation, BUF_SIZE - strlen(operations) - 1);
            }
            write_client(sock, sin, operations);
            printf("✅ OPERATIONS envoyées pour le compte %s\n", id_compte);
            return;
        }
    }
    write_client(sock, sin, "KO\n");
    printf("❌ Authentification incorrecte\n");
   }
}


static int check_if_client_exists(Client *clients, SOCKADDR_IN *csin, int actual) {
   int i = 0;
   for(i = 0; i < actual; i++) {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr && clients[i].sin.sin_port == csin->sin_port) {
        return 1;
      }
   }
   return 0;
}

static Client* get_client(Client *clients, SOCKADDR_IN *csin, int actual) {
   int i = 0;
   for(i = 0; i < actual; i++) {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr && clients[i].sin.sin_port == csin->sin_port) {
        return &clients[i];
      }
   }
   return NULL;
}

static void remove_client(Client *clients, int to_remove, int *actual) {
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(int sock, Client *clients, Client *sender, int actual, const char *buffer, char from_server) {
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++) {
      /* we don't send message to the sender */
      if(sender != &clients[i]) {
        if(from_server == 0) {
            strncpy(message, sender->name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
        }
        strncat(message, buffer, sizeof message - strlen(message) - 1);
        write_client(sock, &clients[i].sin, message);
      }
   }
}

static int server_init_connection(void) {
   /* UDP so SOCK_DGRAM */
   SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET) {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) {
      perror("bind()");
      exit(errno);
   }
   return sock;
}

static void end_connection(int sock) {
   closesocket(sock);
}

static int read_client(SOCKET sock, SOCKADDR_IN *sin, char *buffer) {
   int n = 0;
   socklen_t sinsize = sizeof(*sin);

   if((n = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (SOCKADDR *) sin, &sinsize)) < 0) {
      perror("recvfrom()");
      exit(errno);
   }
   buffer[n] = 0;
   return n;
}

static void write_client(SOCKET sock, SOCKADDR_IN *sin, const char *buffer) {
   if(sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *) sin, sizeof *sin) < 0) {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv) {
   init();
   server_app();
   end();
   return EXIT_SUCCESS;
}