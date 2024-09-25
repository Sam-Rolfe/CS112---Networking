//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      proxy.c 
//*************************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  // For socket        
#include <sys/socket.h> 
#include <netinet/in.h> // For sockaddr_in struct

// ----GLOBAL VARIABLES----------------------------------------------------------------------------
#define PORT 9120
#define BUFFER_SIZE 10000  // Confirm size

//----FUNCTIONS------------------------------------------------------------------------------------


//----MAIN-----------------------------------------------------------------------------------------

int main(void) {
    int proxy_listening_socket, proxy_connection_socket;
    struct sockaddr_in proxy_addr, client_addr; // Struct for handling internet addresses
    socklen_t client_addr_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create socket for proxy
    proxy_listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(proxy_listening_socket < 0) {
        perror("Error creating listening socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket number: %d\n", proxy_listening_socket);

    // Initialize fields of struct for proxy server address
    memset(&proxy_addr, 0, sizeof(proxy_addr)); // Set structure to 0's, ensuring sin_zero is all zeros
    proxy_addr.sin_family = AF_INET;            // Set address family to IPv4
    proxy_addr.sin_addr.s_addr = INADDR_ANY;    // 
    proxy_addr.sin_port = htons(PORT);          // Set port number

    // Bind socket to IP address and port (specificied in proxy_addr)
    if(bind(proxy_listening_socket, (struct sockaddr *) &proxy_addr, sizeof(proxy_addr)) < 0 ){
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections requests
    listen(proxy_listening_socket, 3);
    printf("Listening for incoming connection requests on port %d...\n", PORT);

    while(1) {
        // Accept client connection request and bind to dedicated socket
        proxy_connection_socket = accept(proxy_listening_socket, (struct sockaddr *) &client_addr, &client_addr_size);
        if(proxy_connection_socket  < 0){
            perror("Error creating connection socket");
            exit(EXIT_FAILURE);
        }

        // Read contents from client connection into buffer
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(proxy_connection_socket, buffer, BUFFER_SIZE - 1);
        if(bytes_read < 0) {
            perror("Error reading from connection socket");
            exit(EXIT_FAILURE);
        }

        // Parse contents of client request into HTTP request components
        char method[BUFFER_SIZE], url[BUFFER_SIZE], version[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", method, url, version);

        // Confirm request is GET request (as per project specs)
        if (strncmp(method, "GET", 3) != 0) {
            perror("Proxy server only accepts 'GET' requests");
            close(proxy_connection_socket);
            continue;
        }

        



        printf("Printing buffer: %s\n", buffer);
    }
}