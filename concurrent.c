#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<getopt.h>

#define defaultPort 1587
#define BUFFER_SIZE 1024

int main(int argc,char *argv[]) {
    int server_fd, client_socket, read_size;
    int PORT=defaultPort;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = {0};
    int opt;
    int portarg= 0;
   while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p':
                PORT = atoi(optarg);
                break;
            default:
                fprintf(stderr, "%s [-p port]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Unknown option: %s\n", argv[optind]);
        fprintf(stderr, "%s [-p port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
   
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

   
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Echo server listening on port %d...\n", PORT);

    while (1) {
        int client_addr_len = sizeof(client_addr);

        
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        char client_ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL) {
            perror("Inet_ntop failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        while ((read_size = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
            write(client_socket, buffer, read_size);
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (read_size == 0) {
            printf("Client disconnected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        } else if (read_size == -1) {
            perror("Read failed");
            exit(EXIT_FAILURE);
        }

        close(client_socket);
    }

    close(server_fd);

    return 0;
}
