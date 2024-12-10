#include "list.h"

// Add a user to the user list
struct user_node* addUser(struct user_node *head, int socket, char *username) {
    if (findUser(head, username) == NULL) {
        struct user_node *new_user = (struct user_node*) malloc(sizeof(struct user_node));
        if (new_user == NULL) {
            perror("Memory allocation failed for new user");
            exit(EXIT_FAILURE);
        }
        new_user->socket = socket;
        strcpy(new_user->username, username);
        new_user->dm_connections = NULL;
        new_user->next = head;
        head = new_user;
    } else {
        printf("Username already exists: %s\n", username);
    }
    return head;
}

// Search for a user by username
struct user_node* findUser(struct user_node *head, char* username) {
    struct user_node* current = head;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Remove a user from the user list
struct user_node* removeUser(struct user_node *head, char *username) {
    struct user_node *current = head;
    struct user_node *previous = NULL;

    while (current != NULL && strcmp(current->username, username) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) { // User not found
        return head;
    }

    if (current == head) {
        head = head->next;
    } else {
        previous->next = current->next;
    }

    // Free direct message connections
    struct user_node *dm = current->dm_connections;
    while(dm != NULL) {
        struct user_node *temp = dm;
        dm = dm->next;
        free(temp);
    }

    free(current);
    return head;
}

// Display all users in the user list
void displayUsers(struct user_node *head) {
    struct user_node *current = head;
    while (current != NULL) {
        printf("User: %s\n", current->username);
        current = current->next;
    }
}

// Add a room to the room list
struct room_node* addRoom(struct room_node *head, char *roomname) {
    if (findRoom(head, roomname) == NULL) {
        struct room_node *new_room = (struct room_node*) malloc(sizeof(struct room_node));
        if (new_room == NULL) {
            perror("Memory allocation failed for new room");
            exit(EXIT_FAILURE);
        }
        strcpy(new_room->roomname, roomname);
        new_room->users = NULL;
        new_room->next = head;
        head = new_room;
    } else {
        printf("Room already exists: %s\n", roomname);
    }
    return head;
}

// Search for a room by name
struct room_node* findRoom(struct room_node *head, char* roomname) {
    struct room_node* current = head;
    while (current != NULL) {
        if (strcmp(current->roomname, roomname) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Remove a room from the room list
struct room_node* removeRoom(struct room_node *head, char *roomname) {
    struct room_node *current = head;
    struct room_node *previous = NULL;

    while (current != NULL && strcmp(current->roomname, roomname) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) { // Room not found
        return head;
    }

    if (current == head) {
        head = head->next;
    } else {
        previous->next = current->next;
    }

    // Free the list of users in the room
    struct user_node *user = current->users;
    while(user != NULL) {
        struct user_node *temp = user;
        user = user->next;
        free(temp);
    }

    free(current);
    return head;
}

// List all rooms and append to buffer
void listAllRooms(struct room_node *head, char *buffer) {
    struct room_node *current = head;
    while (current != NULL) {
        strcat(buffer, current->roomname);
        strcat(buffer, "\n");
        current = current->next;
    }
}

// Add a user to a specific room
void addUserToRoom(struct room_node *room, struct user_node *user) {
    // Ensure the user isn't already in the room
    struct user_node *current = room->users;
    while(current != NULL) {
        if(strcmp(current->username, user->username) == 0) {
            // User already exists in the room
            return;
        }
        current = current->next;
    }

    // Add the user to the room's user list
    struct user_node *new_user = (struct user_node*) malloc(sizeof(struct user_node));
    if (new_user == NULL) {
        perror("Memory allocation failed for user in room");
        exit(EXIT_FAILURE);
    }
    strcpy(new_user->username, user->username);
    new_user->socket = user->socket;
    new_user->dm_connections = NULL;
    new_user->next = room->users;
    room->users = new_user;
}

// Remove a user from a specific room
void removeUserFromRoom(struct room_node *room, char *username) {
    struct user_node *current = room->users;
    struct user_node *previous = NULL;

    while (current != NULL && strcmp(current->username, username) != 0) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) { // User not found in the room
        return;
    }

    if (current == room->users) {
        room->users = room->users->next;
    } else {
        previous->next = current->next;
    }

    free(current);
}

// List all users in a specific room and append to buffer
void listUsersInRoom(struct room_node *room, char *buffer) {
    struct user_node *current = room->users;
    while(current != NULL) {
        strcat(buffer, current->username);
        strcat(buffer, "\n");
        current = current->next;
    }
}

// Connect two users for direct messaging
bool connectUsersDM(struct user_node *head, char *user1, char *user2) {
    struct user_node *u1 = findUser(head, user1);
    struct user_node *u2 = findUser(head, user2);

    if(u1 == NULL || u2 == NULL) {
        return false;
    }

    // Ensure users are not already connected
    struct user_node *current = u1->dm_connections;
    while(current != NULL) {
        if(strcmp(current->username, user2) == 0) {
            return false; // Already connected
        }
        current = current->next;
    }

    // Add user2 to user1's DM connections
    struct user_node *new_link1 = (struct user_node*) malloc(sizeof(struct user_node));
    if (new_link1 == NULL) {
        perror("Memory allocation failed for DM connection");
        exit(EXIT_FAILURE);
    }
    strcpy(new_link1->username, u2->username);
    new_link1->socket = u2->socket;
    new_link1->dm_connections = NULL;
    new_link1->next = u1->dm_connections;
    u1->dm_connections = new_link1;

    // Add user1 to user2's DM connections
    struct user_node *new_link2 = (struct user_node*) malloc(sizeof(struct user_node));
    if (new_link2 == NULL) {
        perror("Memory allocation failed for DM connection");
        exit(EXIT_FAILURE);
    }
    strcpy(new_link2->username, u1->username);
    new_link2->socket = u1->socket;
    new_link2->dm_connections = NULL;
    new_link2->next = u2->dm_connections;
    u2->dm_connections = new_link2;

    return true;
}

// Disconnect two users from direct messaging
bool disconnectUsersDM(struct user_node *head, char *user1, char *user2) {
    struct user_node *u1 = findUser(head, user1);
    struct user_node *u2 = findUser(head, user2);

    if(u1 == NULL || u2 == NULL) {
        return false;
    }

    // Remove user2 from user1's DM connections
    struct user_node *current = u1->dm_connections;
    struct user_node *previous = NULL;
    while(current != NULL && strcmp(current->username, user2) != 0) {
        previous = current;
        current = current->next;
    }

    if(current != NULL) {
        if(previous == NULL) {
            u1->dm_connections = current->next;
        } else {
            previous->next = current->next;
        }
        free(current);
    }

    // Remove user1 from user2's DM connections
    current = u2->dm_connections;
    previous = NULL;
    while(current != NULL && strcmp(current->username, user1) != 0) {
        previous = current;
        current = current->next;
    }

    if(current != NULL) {
        if(previous == NULL) {
            u2->dm_connections = current->next;
        } else {
            previous->next = current->next;
        }
        free(current);
    }

    return true;
}

// Check if two users are connected via direct messaging
bool isConnectedDM(struct user_node *head, char *user1, char *user2) {
    struct user_node *u1 = findUser(head, user1);
    if(u1 == NULL) return false;

    struct user_node *current = u1->dm_connections;
    while(current != NULL) {
        if(strcmp(current->username, user2) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}
