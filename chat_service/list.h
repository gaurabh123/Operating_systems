#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Node representing a user in the system
struct user_node {
    char username[30];
    int socket;
    struct user_node *next;
    struct user_node *dm_connections; // Direct message connections
};

// Node representing a room in the system
struct room_node {
    char roomname[30];
    struct room_node *next;
    struct user_node *users; // List of users in the room
};

// User management functions
struct user_node* addUser(struct user_node *head, int socket, char *username);
struct user_node* findUser(struct user_node *head, char* username);
struct user_node* removeUser(struct user_node *head, char *username);
void displayUsers(struct user_node *head);

// Room management functions
struct room_node* addRoom(struct room_node *head, char *roomname);
struct room_node* findRoom(struct room_node *head, char* roomname);
struct room_node* removeRoom(struct room_node *head, char *roomname);
void listAllRooms(struct room_node *head, char *buffer);
void addUserToRoom(struct room_node *room, struct user_node *user);
void removeUserFromRoom(struct room_node *room, char *username);
void listUsersInRoom(struct room_node *room, char *buffer);

// Direct message (DM) management functions
bool connectUsersDM(struct user_node *head, char *user1, char *user2);
bool disconnectUsersDM(struct user_node *head, char *user1, char *user2);
bool isConnectedDM(struct user_node *head, char *user1, char *user2);

#endif // LINKED_LIST_H
