#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;     // User list
extern struct room_node *rooms; // Room list

extern char const *server_MOTD;

// Define delimiters for command parsing
#define delimiters " \n"

// Helper functions for synchronization
void reader_lock() {
    pthread_mutex_lock(&mutex);
    numReaders++;
    if(numReaders == 1){
        pthread_mutex_lock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

void reader_unlock() {
    pthread_mutex_lock(&mutex);
    numReaders--;
    if(numReaders == 0){
        pthread_mutex_unlock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

void writer_lock_func() {
    pthread_mutex_lock(&rw_lock);
}

void writer_unlock_func() {
    pthread_mutex_unlock(&rw_lock);
}

// Function to trim leading and trailing whitespace
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

// Thread function to handle client communication
void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // Socket descriptor
   free(ptr); // Free the dynamically allocated pointer

   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];  // Data buffers
   char tmpbuf[MAXBUFF];  // Temporary buffer for messages
   char cmd[MAXBUFF], username[30];
   char *arguments[80];
   char *token;

   struct node *currentUser;
   struct room_node *currentRoom;

   // Send Welcome Message of the Day
   send(client, server_MOTD, strlen(server_MOTD), 0);

   // Create the guest username
   sprintf(username, "guest%d", client);

   // Acquire write lock to add user
   writer_lock_func();
   head = insertFirstU(head, client, username);
   
   // Add the GUEST to the DEFAULT ROOM (i.e., Lobby)
   currentRoom = findR(rooms, DEFAULT_ROOM);
   if(currentRoom != NULL) {
       currentUser = findU(head, username);
       if(currentUser != NULL) {
           addUserToRoom(currentRoom, currentUser);
       }
   }
   writer_unlock_func();

   while (1) {
       
      if ((received = read(client, buffer, MAXBUFF - 1)) <= 0) {
          // Client disconnected
          printf("Client disconnected: %s\n", username);
          break;
      }
      buffer[received] = '\0'; 
      strcpy(cmd, buffer);  
      strcpy(sbuffer, buffer);
      
      /////////////////////////////////////////////////////
      // Received data from a client

      // 1. Tokenize the input command
      i = 0;
      token = strtok(cmd, delimiters);
      while(token != NULL && i < 80) {
          arguments[i++] = token;
          token = strtok(NULL, delimiters);
      }
      arguments[i] = NULL;

      // Trim whitespace for each argument
      for(int j = 0; j < i; j++) {
          arguments[j] = trimwhitespace(arguments[j]);
      }

      // If no command, continue
      if(arguments[0] == NULL) {
          sprintf(buffer, "\nchat>");
          send(client, buffer, strlen(buffer), 0);
          continue;
      }

      /////////////////////////////////////////////////////
      // 2. Execute command

      if(strcmp(arguments[0], "create") == 0)
      {
         if(i < 2) {
             strcpy(buffer, "Usage: create <room>\nchat>");
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         printf("Create room: %s\n", arguments[1]); 

         // Perform the operation to create room arguments[1]
         writer_lock_func();
         rooms = insertFirstR(rooms, arguments[1]);
         writer_unlock_func();

         sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
         send(client, buffer, strlen(buffer), 0); 
      }
      else if (strcmp(arguments[0], "join") == 0)
      {
         if(i < 2) {
             strcpy(buffer, "Usage: join <room>\nchat>");
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         printf("Join room: %s\n", arguments[1]);  

         // Perform the operation to join room arguments[1]
         writer_lock_func();
         currentRoom = findR(rooms, arguments[1]);
         if(currentRoom == NULL) {
             sprintf(buffer, "Room '%s' does not exist.\nchat>", arguments[1]);
             writer_unlock_func();
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         currentUser = findU(head, username);
         if(currentUser == NULL) {
             sprintf(buffer, "User not found.\nchat>");
             writer_unlock_func();
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         addUserToRoom(currentRoom, currentUser);
         writer_unlock_func();

         sprintf(buffer, "Joined room '%s'.\nchat>", arguments[1]);
         send(client, buffer, strlen(buffer), 0);
      }
      else if (strcmp(arguments[0], "leave") == 0)
      {
         if(i < 2) {
             strcpy(buffer, "Usage: leave <room>\nchat>");
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         printf("Leave room: %s\n", arguments[1]); 

         // Perform the operation to leave room arguments[1]
         writer_lock_func();
         currentRoom = findR(rooms, arguments[1]);
         if(currentRoom == NULL) {
             sprintf(buffer, "Room '%s' does not exist.\nchat>", arguments[1]);
             writer_unlock_func();
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         currentUser = findU(head, username);
         if(currentUser == NULL) {
             sprintf(buffer, "User not found.\nchat>");
             writer_unlock_func();
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         removeUserFromRoom(currentRoom, username);
         writer_unlock_func();

         sprintf(buffer, "Left room '%s'.\nchat>", arguments[1]);
         send(client, buffer, strlen(buffer), 0);
      } 
      else if (strcmp(arguments[0], "connect") == 0)
      {
         if(i < 2) {
             strcpy(buffer, "Usage: connect <user>\nchat>");
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         printf("Connect to user: %s\n", arguments[1]);

         // Perform the operation to connect to user arguments[1]
         writer_lock_func();
         bool success = connectUsers(head, username, arguments[1]);
         writer_unlock_func();

         if(success) {
             sprintf(buffer, "Connected to user '%s'.\nchat>", arguments[1]);
         }
         else {
             sprintf(buffer, "Failed to connect to user '%s'. They may not exist or are already connected.\nchat>", arguments[1]);
         }

         send(client, buffer, strlen(buffer), 0); 
      }
      else if (strcmp(arguments[0], "disconnect") == 0)
      {             
         if(i < 2) {
             strcpy(buffer, "Usage: disconnect <user>\nchat>");
             send(client, buffer, strlen(buffer), 0);
             continue;
         }

         printf("Disconnect from user: %s\n", arguments[1]);

         // Perform the operation to disconnect from user arguments[1]
         writer_lock_func();
         bool success = disconnectUsers(head, username, arguments[1]);
         writer_unlock_func();

         if(success) {
             sprintf(buffer, "Disconnected from user '%s'.\nchat>", arguments[1]);
         }
         else {
             sprintf(buffer, "Failed to disconnect from user '%s'. They may not exist or are not connected.\nchat>", arguments[1]);
         }

         send(client, buffer, strlen(buffer), 0); 
      }                  
      else if (strcmp(arguments[0], "rooms") == 0)
      {
          printf("List all the rooms\n");

          // List all rooms and append to buffer
          writer_lock_func();
          char room_list[MAXBUFF] = "Available rooms:\n";
          listRooms(rooms, room_list);
          writer_unlock_func();

          strcat(room_list, "chat>");
          send(client, room_list, strlen(room_list), 0);                            
      }   
      else if (strcmp(arguments[0], "users") == 0)
      {
          printf("List all the users\n");

          // List all users and append to buffer
          writer_lock_func();
          char user_list[MAXBUFF] = "Connected users:\n";
          struct node *current = head;
          while(current != NULL) {
              strcat(user_list, current->username);
              strcat(user_list, "\n");
              current = current->next;
          }
          writer_unlock_func();

          strcat(user_list, "chat>");
          send(client, user_list, strlen(user_list), 0); 
      }                           
      else if (strcmp(arguments[0], "login") == 0)
      {
          if(i < 2) {
              strcpy(buffer, "Usage: login <username>\nchat>");
              send(client, buffer, strlen(buffer), 0);
              continue;
          }

          char *new_username = arguments[1];
          printf("User '%s' attempting to login as '%s'\n", username, new_username);

          writer_lock_func();

          // Check if new username is already taken
          if(findU(head, new_username) != NULL) {
              sprintf(buffer, "Username '%s' is already taken.\nchat>", new_username);
              writer_unlock_func();
              send(client, buffer, strlen(buffer), 0);
              continue;
          }

          // Find user node
          currentUser = findU(head, username);
          if(currentUser == NULL) {
              sprintf(buffer, "User not found.\nchat>");
              writer_unlock_func();
              send(client, buffer, strlen(buffer), 0);
              continue;
          }

          // Update username in user list
          strcpy(currentUser->username, new_username);

          // Update username in rooms
          struct room_node *r = rooms;
          while(r != NULL) {
              struct node *u = r->users;
              while(u != NULL) {
                  if(u->socket == client) {
                      strcpy(u->username, new_username);
                  }
                  u = u->next;
              }
              r = r->next;
          }

          // Update username in DM connections
          struct node *dm = currentUser->dm_connections;
          while(dm != NULL) {
              // Find other user and update their DM connections
              struct node *other = findU(head, dm->username);
              if(other != NULL) {
                  struct node *other_dm = other->dm_connections;
                  while(other_dm != NULL) {
                      if(strcmp(other_dm->username, username) == 0) {
                          strcpy(other_dm->username, new_username);
                          break;
                      }
                      other_dm = other_dm->next;
                  }
              }
              dm = dm->next;
          }

          // Update username variable for this thread
          strcpy(username, new_username);

          writer_unlock_func();

          sprintf(buffer, "Logged in as '%s'.\nchat>", new_username);
          send(client, buffer, strlen(buffer), 0); 
      } 
      else if (strcmp(arguments[0], "help") == 0 )
      {
          strcpy(buffer, "Available commands:\n");
          strcat(buffer, "login <username> - \"login with username\"\n");
          strcat(buffer, "create <room> - \"create a room\"\n");
          strcat(buffer, "join <room> - \"join a room\"\n");
          strcat(buffer, "leave <room> - \"leave a room\"\n");
          strcat(buffer, "users - \"list all users\"\n");
          strcat(buffer, "rooms -  \"list all rooms\"\n");
          strcat(buffer, "connect <user> - \"connect to user (DM)\"\n");
          strcat(buffer, "disconnect <user> - \"disconnect from user (DM)\"\n");
          strcat(buffer, "exit or logout - \"exit chat\"\n");
          strcat(buffer, "chat>");
          send(client, buffer, strlen(buffer), 0); // Send back to client 
      }
      else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0)
      {
          // Remove the initiating user from all rooms and direct connections, then close the socket descriptor.
          printf("User '%s' is exiting.\n", username);

          writer_lock_func();

          // Remove user from all rooms
          struct room_node *r = rooms;
          while(r != NULL) {
              removeUserFromRoom(r, username);
              r = r->next;
          }

          // Disconnect from all DM connections
          struct node *current_dm = head;
          while(current_dm != NULL) {
              if(strcmp(current_dm->username, username) != 0 && areConnected(head, username, current_dm->username)) {
                  disconnectUsers(head, username, current_dm->username);
              }
              current_dm = current_dm->next;
          }

          // Remove user from user list
          head = deleteU(head, username);

          writer_unlock_func();

          // Close socket
          close(client);
          printf("User '%s' has exited.\n", username);
          break; // Exit the thread
      }                         
      else { 
           /////////////////////////////////////////////////////////////
           // 3. Sending a message
   
           // Format the message
           char formatted_msg[MAXBUFF];
           sprintf(formatted_msg, "::%s> %s\nchat>", username, buffer);

           // Acquire write lock to send messages
           writer_lock_func();

           // Send to all users in the same rooms
           struct room_node *r = rooms;
           while(r != NULL) {
               // Check if user is in the room
               struct node *u = r->users;
               bool in_room = false;
               while(u != NULL) {
                   if(strcmp(u->username, username) == 0) {
                       in_room = true;
                       break;
                   }
                   u = u->next;
               }
               if(in_room) {
                   // Send to all users in the room
                   struct node *recipient = r->users;
                   while(recipient != NULL) {
                       if(recipient->socket != client) { // Don't send to self
                           send(recipient->socket, formatted_msg, strlen(formatted_msg), 0);
                       }
                       recipient = recipient->next;
                   }
               }
               r = r->next;
           }

           // Send to all DM connections
           currentUser = findU(head, username);
           if(currentUser != NULL) {
               struct node *dm = currentUser->dm_connections;
               while(dm != NULL) {
                   send(dm->socket, formatted_msg, strlen(formatted_msg), 0);
                   dm = dm->next;
               }
           }

           writer_unlock_func();
      }

      // Reset buffer
      memset(buffer, 0, sizeof(buffer));
   }

   // User is exiting, perform cleanup

   writer_lock_func();

   // Remove user from all rooms
   struct room_node *r = rooms;
   while(r != NULL) {
       removeUserFromRoom(r, username);
       r = r->next;
   }

   // Disconnect from all DM connections
   struct node *current_dm = head;
   while(current_dm != NULL) {
       if(strcmp(current_dm->username, username) != 0 && areConnected(head, username, current_dm->username)) {
           disconnectUsers(head, username, current_dm->username);
       }
       current_dm = current_dm->next;
   }

   // Remove user from user list
   head = deleteU(head, username);

   writer_unlock_func();

   // Close socket
   close(client);
   printf("User '%s' has disconnected.\n", username);
   return NULL;
}
