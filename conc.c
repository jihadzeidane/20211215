#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>
#include<stdbool.h>
#include<getopt.h>

#define defaultPort 0000
#define BUFFER_SIZE 1024

int authenticateUser(const char* username, const char* password, const char* passwordFile) {
    FILE* file = fopen(passwordFile, "r");
    if (file == NULL) {
        perror("Failed to open password file");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    while (fgets(line, BUFFER_SIZE, file)) {
        line[strcspn(line, "\n")] = '\0';
        char* storedUsername = strtok(line, ":");
        char* storedPassword = strtok(NULL, ":");

        if (strcmp(username, storedUsername) == 0 && strcmp(password, storedPassword) == 0) {
            fclose(file);
            return 1; 
        }
    }

    fclose(file);
    return 0; 
}

int main(int argc, char* argv[]) {
    int server_fd, client_socket, read_size;
    int PORT = defaultPort;
    char* inventory;
    char* passwordUsername;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = {0};
    int opt;
    int portarg = 0;
    while ((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch (opt) {
            case 'd':
                inventory = optarg;
                break;
            case 'p':
                PORT = atoi(optarg);
                break;
            case 'u':
                passwordUsername = optarg;
                break;
            default:
                fprintf(stderr, "%s [-p port]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (passwordUsername == NULL || inventory == NULL || PORT == defaultPort) {
        fprintf(stderr, "Arguments required to run.\n");
        fprintf(stderr, "Pass port using [-p port]\nPass directory using [-d directory]\nPass password using [-u password]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
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

        if ((client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        char client_ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == NULL) {
            perror("Inet_ntop failed");
            exit(EXIT_FAILURE);
        }

        printf("New client connected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        bool accessDenied = false;
        char username[BUFFER_SIZE];
        char password[BUFFER_SIZE];

        while (!accessDenied) {
            write(client_socket, "Please enter your username: ", strlen("Please enter your username: "));
            memset(buffer, 0, BUFFER_SIZE);
            read_size = read(client_socket, buffer, BUFFER_SIZE);
            buffer[strcspn(buffer, "\n")] = '\0'; 
            strcpy(username, buffer);

            write(client_socket, "Please enter your password: ", strlen("Please enter your password: "));
            memset(buffer, 0, BUFFER_SIZE);
            read_size = read(client_socket, buffer, BUFFER_SIZE);
            buffer[strcspn(buffer, "\n")] = '\0'; 
            strcpy(password, buffer);

            int loginResult = authenticateUser(username, password, passwordUsername);
            if (loginResult == 1) {
                write(client_socket, "Login successful!\n", strlen("Login successful!\n"));
                break; 
            } else {
                write(client_socket, "Access denied!\n", strlen("Access denied!\n"));
                write(client_socket, "Please try again.\n", strlen("Please try again.\n"));
                accessDenied = true;
            }
        }

        if (!accessDenied) {
            write(client_socket, "Welcome to the server!\n", strlen("Welcome to the server!\n"));
            write(client_socket, "You are now logged in.\n", strlen("You are now logged in.\n"));
            write(client_socket, "You can use the following commands:\n", strlen("You can use the following commands:\n"));
            write(client_socket, " - list: List filenames and file sizes\n", strlen(" - list: List filenames and file sizes\n"));
            write(client_socket, " - quit: Quit the session\n", strlen(" - quit: Quit the session\n"));

            bool loggedIn = true;
            while (loggedIn) {
                write(client_socket, "Please enter a command: \n", strlen("Please enter a command: \n"));
                memset(buffer, 0, BUFFER_SIZE);
                read_size = read(client_socket, buffer, BUFFER_SIZE);
                buffer[strcspn(buffer, "\n")] = '\0';

                
                if (strcmp(buffer, "list") == 0) {
                    DIR* directory;
                    struct dirent* file;

                    directory = opendir(inventory);
                    if (directory == NULL) {
                        write(client_socket, "Failed to open directory.\n", strlen("Failed to open directory.\n"));
                    } else {
                        while ((file = readdir(directory)) != NULL) {
                            if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
                                char fileDetails[BUFFER_SIZE];
                                sprintf(fileDetails, "%s %ld\n", file->d_name, file->d_reclen);
                                write(client_socket, fileDetails, strlen(fileDetails));
                            }
                        }
                        closedir(directory);
                        write(client_socket, ".\n", strlen(".\n"));
                    }
                } else if (strcmp(buffer, "quit") == 0) {
                    write(client_socket, "Goodbye!\n", strlen("Goodbye!\n"));
                    loggedIn = false;
                } else {
                    write(client_socket, "Unrecognized command. Please try again.\n", strlen("Unrecognized command. Please try again.\n"));
                }
            }
        }

        close(client_socket);
    }

    close(server_fd);

    return 0;
}
