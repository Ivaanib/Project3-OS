#include "server.h"

#define DEFAULT_ROOM "Lobby"

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

extern struct node *head;
extern char *server_MOTD;

struct room {
    char name[50];           // Room name
    struct node *users;      // Linked list of users in the room
    struct room *next;       // Pointer to the next room
};


char *trimwhitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';
    return str;
}

void *client_receive(void *ptr) {
    int client = *(int *)ptr;  // socket

    int received, i;
    char buffer[MAXBUFF], sbuffer[MAXBUFF];  // data buffer
    char tmpbuf[MAXBUFF];  // temp buffer
    char cmd[MAXBUFF], username[20];
    char *arguments[80];

    send(client, server_MOTD, strlen(server_MOTD), 0);  // Send MOTD

    // Creating the guest username
    sprintf(username, "guest%d", client);
    head = insertFirstU(head, client, username);

    // Add the GUEST to the DEFAULT ROOM
    printf("Added guest '%s' to Lobby\n", username);

    while (1) {
        if ((received = read(client, buffer, MAXBUFF)) != 0) {
            buffer[received] = '\0';

            strcpy(cmd, buffer);
            strcpy(sbuffer, buffer);

            arguments[0] = strtok(cmd, delimiters);
            i = 0;
            while (arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, delimiters);
                if (arguments[i - 1]) strcpy(arguments[i - 1], trimwhitespace(arguments[i - 1]));
            }

            if (strcmp(arguments[0], "create") == 0) {
                pthread_mutex_lock(&rw_lock);
                struct room *new_room = malloc(sizeof(struct room));
                strcpy(new_room->name, arguments[1]);
                new_room->users = NULL;
                new_room->next = head;
                head = new_room;
                pthread_mutex_unlock(&rw_lock);
                printf("Created room: %s\n", arguments[1]);
                sprintf(buffer, "Room '%s' created.\nchat>", arguments[1]);
                send(client, buffer, strlen(buffer), 0);
            } else if (strcmp(arguments[0], "join") == 0) {
                pthread_mutex_lock(&rw_lock);
                // (Implementation similar to create room for joining)
                pthread_mutex_unlock(&rw_lock);
            } else if (strcmp(arguments[0], "leave") == 0) {
                pthread_mutex_lock(&rw_lock);
                // (Implementation for leaving a room)
                pthread_mutex_unlock(&rw_lock);
            } else if (strcmp(arguments[0], "users") == 0) {
                pthread_mutex_lock(&rw_lock);
                // (Implementation for listing users)
                pthread_mutex_unlock(&rw_lock);
            } else if (strcmp(arguments[0], "exit") == 0) {
                pthread_mutex_lock(&rw_lock);
                // (Implementation for exiting the chat)
                pthread_mutex_unlock(&rw_lock);
            }
        }
    }
    return NULL;
}