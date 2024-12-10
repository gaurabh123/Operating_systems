
#include "server.h"

int chat_serv_sock_fd; // Server socket

/////////////////////////////////////////////
// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE

int numReaders = 0; // Keep count of the number of readers

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex lock
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // Read/Write lock

/////////////////////////////////////////////

char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head = NULL; // User list
struct room_node *rooms = NULL; // Room list

int main(int argc, char **argv) {

   // Set up SIGINT handler for graceful shutdown
   signal(SIGINT, sigintHandler);

   //////////////////////////////////////////////////////
   // Create the default room for all clients to join when initially connecting
   //////////////////////////////////////////////////////

   // Initialize rooms with default room "Lobby"
   rooms = insertFirstR(rooms, DEFAULT_ROOM);
   if(rooms == NULL) {
       printf("Failed to create default room\n");
       exit(1);
   }

   // Open server socket
   chat_serv_sock_fd = get_server_socket();

   // Start listening for connections
   if(start_server(chat_serv_sock_fd, BACKLOG) == -1) {
      printf("Start server error\n");
      exit(1);
   }
   
   printf("Server Launched! Listening on PORT: %d\n", PORT);
    
   // Main execution loop
   while(1) {
      // Accept a connection and start a thread
      int new_client = accept_client(chat_serv_sock_fd);
      if(new_client != -1) {
         pthread_t new_client_thread;
         int *pclient = malloc(sizeof(int));
         if(pclient == NULL) {
             perror("Failed to allocate memory for client socket");
             exit(EXIT_FAILURE);
         }
         *pclient = new_client;
         pthread_create(&new_client_thread, NULL, client_receive, pclient);
         pthread_detach(new_client_thread); // Detach thread to reclaim resources when done
      }
   }

   close(chat_serv_sock_fd);
}

// Create and return the server socket
int get_server_socket(char *hostname, char *port) {
    int opt = 1;   
    int master_socket;
    struct sockaddr_in address; 
    
    // Create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("Socket failed");   
        exit(EXIT_FAILURE);   
    }   
    
    // Set master socket to allow multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    {   
        perror("Setsockopt");   
        exit(EXIT_FAILURE);   
    }   
    
    // Type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    // Bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("Bind failed");   
        exit(EXIT_FAILURE);   
    }   

   return master_socket;
}

// Start listening on the server socket
int start_server(int serv_socket, int backlog) {
   int status = 0;
   if ((status = listen(serv_socket, backlog)) == -1) {
      perror("Listen");
      return -1;
   }
   return status;
}

// Accept a new client connection
int accept_client(int serv_sock) {
   int reply_sock_fd = -1;
   socklen_t sin_size = sizeof(struct sockaddr_storage);
   struct sockaddr_storage client_addr;

   // Accept a connection request from a client
   if ((reply_sock_fd = accept(serv_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
      perror("Accept");
   }
   return reply_sock_fd;
}

/* Handle SIGINT (CTRL+C) for graceful shutdown */
void sigintHandler(int sig_num) {
   printf("\nGracefully shutting down the server...\n");

   //////////////////////////////////////////////////////////
   // Closing client sockets and freeing memory from user list
   //////////////////////////////////////////////////////////

   // Acquire write lock to modify user and room lists
   pthread_mutex_lock(&rw_lock);
   
   // Close all client sockets and free user list
   struct node *current = head;
   while(current != NULL) {
       close(current->socket);
       current = current->next;
   }

   // Free user list
   while(head != NULL) {
       struct node *temp = head;
       head = head->next;
       // Free DM connections
       struct node *dm = temp->dm_connections;
       while(dm != NULL) {
           struct node *dmtmp = dm;
           dm = dm->next;
           free(dmtmp);
       }
       free(temp);
   }

   // Free rooms and their user lists
   while(rooms != NULL) {
       struct room_node *rtemp = rooms;
       rooms = rooms->next;
       // Free users in the room
       struct node *u = rtemp->users;
       while(u != NULL) {
           struct node *utemp = u;
           u = u->next;
           free(utemp);
       }
       free(rtemp);
   }

   pthread_mutex_unlock(&rw_lock);

   printf("--------CLOSING ACTIVE USERS AND ROOMS--------\n");

   close(chat_serv_sock_fd);
   exit(0);
}
