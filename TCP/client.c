#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "client.h"


static void init(void) {
    printf("✅ Initialisation du serveur...\n");
}

static void end(void) {
    printf("🛑 Fermeture du serveur...\n");
}

static void client_app(const char *address, const char *name) {
   SOCKET sock = client_init_connection(address);
   char buffer[BUF_SIZE];

   //fd_set est une structure permettant de gerer des descripteurs de socket
   //via un champ representant un tableau de bits 
   //source : https://stackoverflow.com/questions/18952564/understanding-fd-set-in-unix-sys-select-h
   fd_set rdfs; 

   /* send our name */
   write_server(sock, name);
   //printf("✅ Nom du client envoyé : %s\n", name);
   while(1) {
      /* clear the set rfds */
      FD_ZERO(&rdfs);

      /* add keyboard STDIN_FILENO to the set rfds */
      /* cela signifie que la fonction select() va surveiller lorsqu'on va ecrire sur le clavier*/
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the socket in the set rfds*/
      FD_SET(sock, &rdfs);

      //select va surveiller tous les descripteurs de fichiers de l'ensemble rfds
      //toutefois elle ne surveille par les ecritures et les exceptions
      //pas de timeout
      //le programme est bloqué jusqu'à ce qu'un evenement se produise comme une entrée clavier detectee ou bien une reception de donnees 
      if(select(sock + 1, &rdfs, NULL, NULL, NULL) == -1) {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      /*FD_ISSET vérifie si STDIN_FILENO est contenu dans rfds après l'appel à select*/
      if(FD_ISSET(STDIN_FILENO, &rdfs)) {
         /*Lit les caracteres saisies au clavier et les stocke dans buffer*/
         fgets(buffer, BUF_SIZE - 1, stdin);
         {
            char *p = NULL;
            p = strstr(buffer, "\n");
            if(p != NULL) {
               *p = 0;
            }else {
               /* fclean */
               buffer[BUF_SIZE - 1] = 0;
            }
         }
         /*Envoie le contenu du buffer au serveur*/
         write_server(sock, buffer);

      }/*FD_ISSET vérifie si des donnees sont disponibles sur le socket*/
      else if(FD_ISSET(sock, &rdfs)) {
         /*Lecture du nombre d'octet recus depuis le socket*/
         int n = read_server(sock, buffer);
         /* server down */
         if(n == 0) {
            printf("🔌Server disconnected !\n");
            break;
         }
         puts(buffer);
      }
   }
   end_connection(sock);
}

static int client_init_connection(const char *address) {
   //En Linux, tout fichier est representé par un descripteur de fichier, qui est un entier non negatif.
   //Ici, la fonction socket() permet d'attribuer un entier a la variable sock  
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 }; //Initialise une adresse IPv4 à 0 pour eviter les erreurs
   printf("✅ Socket créé avec succès. Descripteur du socket : %d\n", sock);

   struct hostent *hostinfo; //structure qui contient des informations sur un hôte comme le nom, l'adresse IP, etc.

   if(sock == INVALID_SOCKET) {
      printf("🛑 Le Socket n'a pas pu être créé.\n");
      perror("socket()");
      exit(errno);
   }

   //elle peut prendre une str (www.google.com) ou bien une adresse IP (127.0.0.1)
   hostinfo = gethostbyname(address); 
   if (hostinfo == NULL) {
      fprintf (stderr, "Unknown host %s.\n", address);
      exit(EXIT_FAILURE);
   }

   sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
   sin.sin_port = htons(PORT); 
   sin.sin_family = AF_INET; //Famille de protocoles : IPv4

   printf("🔄 Tentative de connexion au serveur %s sur le port %d...\n", address, PORT);
   if (connect(sock, (SOCKADDR *)&sin, sizeof(sin)) == SOCKET_ERROR) {
    perror("❌ Erreur lors de la connexion au serveur");
    exit(errno);
   }else {
      printf("✅ Connexion établie avec le serveur !\n");
   }

   return sock;
}

static void end_connection(int sock) {
   closesocket(sock);
}

static int read_server(SOCKET sock, char *buffer) {
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) {
      perror("recv()");
      exit(errno);
   }

   buffer[n] = 0;

   return n;
}

static void write_server(SOCKET sock, const char *buffer) {
   if(send(sock, buffer, strlen(buffer), 0) < 0) {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv) {
   if(argc < 2) {
      printf("Usage : %s [address] [pseudo]\n", argv[0]);
      return EXIT_FAILURE;
   }

   init();

   client_app(argv[1], argv[2]);

   end();

   return EXIT_SUCCESS;
}