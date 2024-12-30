#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#define CRLF            "\r\n"
#define PORT            1977
#define MAX_CLIENTS     100
#define BUF_SIZE        1024

#include "client.h"

typedef struct Operation_t {
    char type_operation[50];    //Type de l'operation (AJOUT, RETRAIT, SOLDE, OPERATIONS)
    char date_operation[50];    //Date et heure au format "YYYY-MM-DD HH:MM:SS"
    int montant_operation;      //Montant de l'operation
}Operation;

typedef struct Compte_t {
    char id_client[BUF_SIZE]; //comme un nom d'utilisateur
    char id_compte[BUF_SIZE]; //numero de compte bancaire
    char password[BUF_SIZE];
    int solde;
    Operation operations[10]; //Tableau stockant les 10 dernieres operations
    int nb_operations; //Nombre d'operations effectuées
}Compte;


static void init(void);
static void end(void);
static void server_app(void);
void obtenir_date_heure(char *buffer, size_t buffer_size);
static void traiter_commande(Client *client, Compte *comptes, int nb_comptes, const char *commande);
static int server_init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);

#endif /* guard */