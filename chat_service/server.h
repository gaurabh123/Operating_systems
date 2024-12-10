#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "list.h"

#define PORT 8888
#define BACKLOG 10
#define MAXBUFF 2096

#define DEFAULT_ROOM "Lobby"

// Function prototypes
int get_server_socket();
int start_server(int serv_socket, int backlog);
int accept_client(int serv_sock);
void *client_receive(void *ptr);
void sigintHandler(int sig_num);

// Global variables
extern int chat_serv_sock_fd; // Server socket
extern struct node *head;     // User list
extern struct room_node *rooms; // Room list

// Synchronization primitives
extern int numReaders;
extern pthread_mutex_t mutex;
extern pthread_mutex_t rw_lock;

// Message of the Day
extern char const *server_MOTD;

#endif // SERVER_H
