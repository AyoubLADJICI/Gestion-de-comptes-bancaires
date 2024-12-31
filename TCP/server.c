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
   printf("🟢 Serveur en écoute sur le port %d...\n", PORT);
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];

   fd_set rdfs;

   while(1) {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++) {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1) {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs)) {
         /* stop process when type on keyboard */
         break;
      }else if(FD_ISSET(sock, &rdfs)) {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         socklen_t sinsize = sizeof(csin);
         /* Nouveau socket qui sera utilise pour la communication avec le client */
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR) {
            perror("accept()");
            continue;
         }
         //Slide 60 : ntohs permet de convertir un numéro de port dans l’ordre d’octet réseau en numéro de port IP dans l’ordre d’octet de l’hôte
         //Slide 95 : inet_ntoa permet de convertir l'adresse de l'hôte en une chaine de caracteres IPv4
         //Source : https://www.csd.uoc.gr/%7Ehy556/material/tutorials/cs556-3rd-tutorial.pdf
         printf("✅ Nouveau client connecté depuis %s:%d\n", inet_ntoa(csin.sin_addr), ntohs(csin.sin_port));
         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1) {
            /* disconnected */
            continue;
         }
         char welcome_message[BUF_SIZE];
         snprintf(welcome_message, BUF_SIZE, "🟢 Bienvenue %.50s sur le serveur bancaire !\n", clients[i].name);

         char requete_message[BUF_SIZE] = 
            "Voici les commandes disponibles :\n"
            "- AJOUT <id_client> <id_compte> <password> <somme>\n"
            "- RETRAIT <id_client> <id_compte> <password> <somme>\n"
            "- SOLDE <id_client> <id_compte> <password>\n"
            "- OPERATIONS <id_client> <id_compte> <password>\n"
            "Veuillez respecter la syntaxe des commandes.\n";

         /* Envoyer le message de bienvenue */
         write_client(csock, welcome_message);
         /* Envoyer le message des commandes disponibles */
         write_client(csock, requete_message);
         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);
         Client c = { csock };
         strncpy(c.name, buffer, BUF_SIZE - 1);
         clients[actual] = c;
         printf("📤 Diffusion du message : %s s'est connecté \n", clients[i].name);
         actual++;
      }else {
         int i = 0;
         for(i = 0; i < actual; i++) {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs)) {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               /* client disconnected */
               if(c == 0) {
                  printf("❌ Client déconnecté : %s\n", clients[i].name);
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
               }else {
                  traiter_commande(&clients[i], comptes, 3, buffer);
                  //printf("📩 %s vient d'effectuer une operation dans notre banque ! \n", clients[i].name);
                  //printf("📩 Message reçu de %s : %s\n", clients[i].name, buffer);
                  /*la ligne suivante permet de diffuser le message à tous les clients connectés*/
                  /*on la commente pour ne pas diffuser les informations du compte aux autres clients*/
                  //send_message_to_all_clients(clients, client, actual, buffer, 0);
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
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

static void traiter_commande(Client *client, Compte *comptes, int nb_comptes, const char *commande) {
    char requete[10], id_client[BUF_SIZE], id_compte[BUF_SIZE], password[BUF_SIZE];
    int somme;

    // Découpage de la commande
    sscanf(commande, "%s %s %s %s %d", requete, id_client, id_compte, password, &somme);

    if (strcmp(requete, "AJOUT") == 0) {
        for (int i = 0; i < nb_comptes; i++) {
            //On verifie si les informations de la requete du client correpondent bien à un compte existant
            if (strcmp(comptes[i].id_client, id_client) == 0 && strcmp(comptes[i].id_compte, id_compte) == 0 && strcmp(comptes[i].password, password) == 0) {
                comptes[i].solde += somme;
                //Ajouter une nouvelle opération
                if (comptes[i].nb_operations < 10) {
                    strcpy(comptes[i].operations[comptes[i].nb_operations].type_operation, "AJOUT");
                    obtenir_date_heure(comptes[i].operations[comptes[i].nb_operations].date_operation, 50);
                    comptes[i].operations[comptes[i].nb_operations].montant_operation = somme;
                    comptes[i].nb_operations++;
                } else {
                    //Décaler les opérations si le tableau est plein
                    for (int j = 1; j < 10; j++) {
                        comptes[i].operations[j - 1] = comptes[i].operations[j];
                    }
                    strncpy(comptes[i].operations[9].type_operation, "AJOUT", 50);
                    obtenir_date_heure(comptes[i].operations[9].date_operation, 50);
                    comptes[i].operations[9].montant_operation = somme;
                }
                //Répondre au client
                write_client(client->sock, "OK");
                printf("✅ AJOUT réussi : %d€ viennent d'être ajoutées au compte %s\n", somme, id_compte);
                return;
            }
        }
        // Si aucun compte ne correspond
        write_client(client->sock, "KO\n");
        printf("❌ authentification incorrecte\n");
    }else if (strcmp(requete, "RETRAIT") == 0) {
         for (int i = 0; i < nb_comptes; i++) {
               //On verifie si les informations de la requete du client correpondent bien a un compte existant
               if (strcmp(comptes[i].id_client, id_client) == 0 && strcmp(comptes[i].id_compte, id_compte) == 0 && strcmp(comptes[i].password, password) == 0) {
                  if (comptes[i].solde >= somme) {
                     comptes[i].solde -= somme;
                     //Ajouter une nouvelle operation
                     if (comptes[i].nb_operations < 10) {
                           strcpy(comptes[i].operations[comptes[i].nb_operations].type_operation, "RETRAIT");
                           obtenir_date_heure(comptes[i].operations[comptes[i].nb_operations].date_operation, 50);
                           comptes[i].operations[comptes[i].nb_operations].montant_operation = -somme;
                           comptes[i].nb_operations++;
                     } else {
                           //Decaler les opérations si le tableau est plein
                           for (int j = 1; j < 10; j++) {
                              comptes[i].operations[j - 1] = comptes[i].operations[j];
                           }
                           strncpy(comptes[i].operations[9].type_operation, "RETRAIT", 50);
                           obtenir_date_heure(comptes[i].operations[9].date_operation, 50);
                           comptes[i].operations[9].montant_operation = somme;
                     }
                     //Répondre au client
                     write_client(client->sock, "OK");
                     printf("✅ RETRAIT réussi : %d€ viennent d'être retirées du compte %s\n", somme, id_compte);
                     return;
                  } else {
                     //Répondre au client
                     write_client(client->sock, "KO"); 
                     printf("❌ RETRAIT échoué : solde insuffisant\n");
                     printf("%s, votre solde est de %d€, vous ne pouvez pas retirer %d€\n",id_client, comptes[i].solde, somme);
                     return;
                  }
               }
         }
         // Si aucun compte ne correspond
         write_client(client->sock, "KO\n");
         printf("❌ Authentification incorrecte\n");
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
                  write_client(client->sock, reponse);
                  printf("✅ SOLDE : %s, votre solde est de %d€\n", id_client, comptes[i].solde);
                  return;
               }
         }
         // Si aucun compte ne correspond
         write_client(client->sock, "KO\n");
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
                  write_client(client->sock, operations);
                  printf("✅ OPERATIONS envoyées pour le compte %s\n", id_compte);
                  return;
            }
         }
         write_client(client->sock, "KO");
         printf("❌ Authentification incorrecte pour OPERATIONS\n");
    }

}  


static void clear_clients(Client *clients, int actual) {
   int i = 0;
   for(i = 0; i < actual; i++) {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual) {
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server) {
   printf("📤 Diffusion du message : %s\n", buffer);
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++) {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock) {
         if(from_server == 0) {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

static int server_init_connection(void) {
   //En Linux, tout fichier est representé par un descripteur de fichier, qui est un entier non negatif.
   //Ici, la fonction socket() permet d'attribuer un entier a la variable sock
   //AF_INET permet d'employer les protocoles Internet IPv4
   //SOCK_STREAM permet une communication garantissant integrite et un mode connecte
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 }; //Initialise une adresse IPv4 à 0 pour eviter les erreurs
   printf("✅ Socket créé avec succès. Descripteur du socket : %d\n", sock);
   if(sock == INVALID_SOCKET) {
      printf("🛑 Le Socket n'a pas pu être créé.\n");
      perror("socket()");
      exit(errno);
   }
   /*INADDR_ANY = 0x00000000  */
   /*cela signifie que le serveur va accepter n'importe des connexions provenant de n'importe quelle adresse IP*/
   /*source : http://www.castaglia.org/proftpd/doc/devel-guide/src/include/inet.h.html*/
   sin.sin_addr.s_addr = htonl(INADDR_ANY); 
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;
   /*bind() permet de lier un socket avec une structure sockaddr.*/
   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR){
      perror("bind()");
      exit(errno);
   }
   /*listen() definit la taille de la file de connexions en attente pour notre socket*/
   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
      perror("listen()");
      exit(errno);
   }
   return sock;
}

static void end_connection(int sock) {
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer) {
   int n = 0;
   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }
   buffer[n] = 0;
   return n;
}

static void write_client(SOCKET sock, const char *buffer) {
   if(send(sock, buffer, strlen(buffer), 0) < 0) {
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