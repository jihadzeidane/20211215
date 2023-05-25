#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<getopt.h>
#include<stdbool.h>

#define defaultPort 0000
#define BUFFER_SIZE 1024
bool authenticateUser(const char *username, const char *password, const char *passwordFile) {
    FILE *file = fopen(passwordFile, "r");
    if (file == NULL) {
        perror("Failed to open password file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    char *storedUsername;
    char *storedPassword;

    while (fgets(line, sizeof(line), file)) {
        storedUsername = strtok(line, ":");
        storedPassword = strtok(NULL, ":");
        storedPassword[strcspn(storedPassword, "\n")] = '\0';
        if (storedUsername != NULL && storedPassword != NULL &&
            strcmp(username, storedUsername) == 0 && strcmp(password, storedPassword) == 0) {
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}

int main(int argc,char *argv[]) {
    int server_fd, client_socket, read_size;
    int PORT=defaultPort;
    char *inventory;
    char *passwordUsername;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE] = {0};
    int opt;
    int portarg= 0;
   while ((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch (opt) {
             case 'd':
            inventory=optarg;
            break;
            case 'p':
            PORT = atoi(optarg);
            break;
           
            case 'u':
             passwordUsername=optarg;
                break;
            default:
            fprintf(stderr, "%s [-p port]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

   
    if(passwordUsername==NULL||inventory==NULL||PORT==defaultPort){
        fprintf(stderr,"Arguements required to run.\n");
        fprintf(stderr,"Pass port using [-p port]\nPass directory using [-d directory]\nPass password using [-u password]\n",argv[0]);
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

    printf("Server listening on port %d...\n", PORT);

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
       
        write(client_socket,"........Welcome..........",strlen("........Welcome.........."));
while (1) {
   

    printf("New client connected: %s:%d\n", client_ip, ntohs(client_addr.sin_port));
    
   bool loginSuccessful = false;
    while (!loginSuccessful) {
        bool accessDenied = false;

       
        write(client_socket, "Please enter your username: ", strlen("Please enter your username: "));
        memset(buffer, 0, BUFFER_SIZE);
        read_size = read(client_socket, buffer, BUFFER_SIZE);
        buffer[strcspn(buffer, "\n")] = '\0'; 
        char* username = strdup(buffer);

        write(client_socket, "Please enter your password: ", strlen("Please enter your password: "));
        memset(buffer, 0, BUFFER_SIZE);
        read_size = read(client_socket, buffer, BUFFER_SIZE);
        buffer[strcspn(buffer, "\n")] = '\0'; 
        char* password = strdup(buffer);

       
        loginSuccessful = authenticateUser(username, password, passwordUsername);
        if (loginSuccessful) {
            write(client_socket, "Login successful!\n", strlen("Login successful!\n"));
            break; 
        } else {
            write(client_socket, "Access denied!\n", strlen("Access denied!\n"));
            write(client_socket, "Please try again.\n", strlen("Please try again.\n"));
            accessDenied = true;
        }

        
        free(username);
        free(password);

        if (accessDenied) {
            continue; 
        }
    }

   
        

    // ...
}


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
