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


#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100
#define BUF_SIZE    1024

#include "client.h"

// Gestion des comptes bancaires
typedef struct {
    char type_operation[50];
    char date_operation[50];
    int montant_operation;
} Operation;

typedef struct {
    char id_client[BUF_SIZE];
    char id_compte[BUF_SIZE];
    char password[BUF_SIZE];
    int solde;
    int nb_operations;
    Operation operations[10];
} Compte;

// Prototypes supplémentaires
static void traiter_commande(SOCKET sock, SOCKADDR_IN *sin, Compte *comptes, int nb_comptes, const char *commande);
static void obtenir_date_heure(char *buffer, size_t buffer_size);
static void init(void);
static void end(void);
static void server_app(void);
static int server_init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, SOCKADDR_IN *csin, char *buffer);
static void write_client(SOCKET sock, SOCKADDR_IN *csin, const char *buffer);
static void send_message_to_all_clients(int sock, Client *clients, Client *client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static int check_if_client_exists(Client *clients, SOCKADDR_IN *csin, int actual);
static Client* get_client(Client *clients, SOCKADDR_IN *csin, int actual);

#endif /* guard */