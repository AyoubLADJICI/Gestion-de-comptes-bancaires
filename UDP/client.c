#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "client.h"

static void init(void) {
    printf("✅ Initialisation du serveur...\n");
}

static void end(void) {
    printf("🛑 Fermeture du serveur...\n");
}

static void client_app(const char *address, const char *name) {
   SOCKADDR_IN sin = { 0 };
   SOCKET sock = client_init_connection(address, &sin);
   char buffer[BUF_SIZE];

   fd_set rdfs;

   /* send our name */
   write_server(sock, &sin, name);

   while(1) {
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the socket */
      FD_SET(sock, &rdfs);

      if(select(sock + 1, &rdfs, NULL, NULL, NULL) == -1) {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs)) {
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
         write_server(sock, &sin, buffer);
      }else if(FD_ISSET(sock, &rdfs)) {
         int n = read_server(sock, &sin, buffer);
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

static int client_init_connection(const char *address, SOCKADDR_IN *sin) {
   /* UDP so SOCK_DGRAM */
   SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   struct hostent *hostinfo;

   if(sock == INVALID_SOCKET) {
      perror("socket()");
      exit(errno);
   }

   hostinfo = gethostbyname(address);
   if (hostinfo == NULL) {
      fprintf (stderr, "Unknown host %s.\n", address);
      exit(EXIT_FAILURE);
   }

   sin->sin_addr = *(IN_ADDR *) hostinfo->h_addr;
   sin->sin_port = htons(PORT);
   sin->sin_family = AF_INET;

   return sock;
}

static void end_connection(int sock) {
   closesocket(sock);
}

static int read_server(SOCKET sock, SOCKADDR_IN *sin, char *buffer) {
   int n = 0;
   socklen_t sinsize = sizeof(*sin);

   if((n = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (SOCKADDR *) sin, &sinsize)) < 0) {
      perror("recvfrom()");
      exit(errno);
   }
   buffer[n] = 0;
   return n;
}

static void write_server(SOCKET sock, SOCKADDR_IN *sin, const char *buffer) {
   if(sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *) sin, sizeof *sin) < 0) {
      perror("sendto()");
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