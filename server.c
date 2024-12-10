#include "server.h"

int chat_serv_sock_fd; // server socket

// USE THESE LOCKS AND COUNTER TO SYNCHRONIZE
int numReaders = 0; // keep count of the number of readers
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex lock
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;  // read/write lock

char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head = NULL;

struct room {
    char name[50];
    struct node *users;
    struct room *next;
};

// Initialize default room
void initialize_default_room() {
    pthread_mutex_lock(&rw_lock);
    struct  room *default_room = malloc(sizeof(struct room));
    strcpy(default_room->name, "Lobby");
    default_room->users = NULL;
    default_room->next = NULL;
    head = insertRoom(head, default_room);
    pthread_mutex_unlock(&rw_lock);
}

// Main function
int main(int argc, char **argv) {
    signal(SIGINT, sigintHandler);

    initialize_default_room(); // Create Lobby room

    // Open server socket
    chat_serv_sock_fd = get_server_socket();

    // Step 3: get ready to accept connections
    if (start_server(chat_serv_sock_fd, BACKLOG) == -1) {
        printf("start server error\n");
        exit(1);
    }

    printf("Server Launched! Listening on PORT: %d\n", PORT);

    // Main execution loop
    while (1) {
        int new_client = accept_client(chat_serv_sock_fd);
        if (new_client != -1) {
            pthread_t new_client_thread;
            pthread_create(&new_client_thread, NULL, client_receive, (void *)&new_client);
        }
    }

    close(chat_serv_sock_fd);
}

// Create server socket
int get_server_socket(char *hostname, char *port) {
    int opt = TRUE;
    int master_socket;
    struct sockaddr_in address;

    // Create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set master socket to allow multiple connections
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return master_socket;
}

// Start server
int start_server(int serv_socket, int backlog) {
    int status = 0;
    if ((status = listen(serv_socket, backlog)) == -1) {
        printf("socket listen error\n");
    }
    return status;
}

// Accept client connection
int accept_client(int serv_sock) {
    int reply_sock_fd = -1;
    socklen_t sin_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage client_addr;

    // Accept a connection request from a client
    if ((reply_sock_fd = accept(serv_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
        printf("socket accept error\n");
    }
    return reply_sock_fd;
}

// Handle SIGINT (CTRL+C)
void sigintHandler(int sig_num) {
    printf("\nGraceful shutdown initiated.\n");

    pthread_mutex_lock(&rw_lock);

    // Free all rooms
    while (head) {
        struct room *room_to_free = head;
        head = head->next;

        // Free users in the room
        struct node *user = room_to_free->users;
        while (user) {
            struct node *user_to_free = user;
            user = user->next;
            free(user_to_free);
        }

        free(room_to_free);
    }

    pthread_mutex_unlock(&rw_lock);

    printf("All rooms and users freed.\n");
    close(chat_serv_sock_fd);
    printf("Server socket closed.\n");
    exit(0);
}