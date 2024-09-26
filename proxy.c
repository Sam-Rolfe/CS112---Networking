//----HEADER---------------------------------------------------------------------------------------
// Author:      Sam Rolfe
// Date:        September 2024
// Script:      proxy.c 
// Usage:       ./a.out 9210
//*************************************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>  // For socket        
#include <sys/socket.h> 
#include <netinet/in.h> // Provides sockaddr_in struct
#include <netdb.h>      // Provides hostent struct

// ----GLOBAL VARIABLES----------------------------------------------------------------------------
#define DEFAULT_SERVER_PORT 80
#define BUFFER_SIZE 10000  // Confirm size

//----FUNCTIONS------------------------------------------------------------------------------------


//----MAIN-----------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    // Declare variables
    int PROXY_PORT;
    int proxy_listening_socket, proxy_connection_socket;
    struct sockaddr_in proxy_addr, client_addr; // Struct for handling internet addresses
    socklen_t client_addr_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Get port number from program argument
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    PROXY_PORT = atoi(argv[1]);
    printf("PROXY_PORT: %d\n", PROXY_PORT);

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
    proxy_addr.sin_addr.s_addr = INADDR_ANY;    // Set IP address to all IP addresses of machine
    proxy_addr.sin_port = htons(PROXY_PORT);          // Set port number

    // Set socket options to allow reuse of the address (fixes "address already in use" bug).
    // Consider removing for submission
    int opt = 1;
    if (setsockopt(proxy_listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(proxy_listening_socket);
        exit(EXIT_FAILURE);
    }   

    // Bind socket to IP address and port (specificied in proxy_addr)
    if(bind(proxy_listening_socket, (struct sockaddr *) &proxy_addr, sizeof(proxy_addr)) < 0 ){
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections requests on "listening socket"
    listen(proxy_listening_socket, 3);
    printf("Listening for incoming connection requests on port %d...\n", PROXY_PORT);

    while(1) {
        // Yield CPU and await connection request. Upon reception,
        // bind to dedicated socket
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

        printf("Printing client HTTP request: \n%s\n", buffer);

        // Get the hostname, path, and port number (if present) from URL
        int SERVER_PORT = DEFAULT_SERVER_PORT;
        char hostname[BUFFER_SIZE], path[BUFFER_SIZE];
        sscanf(url, "http://%[^:/]:%d%s", hostname, &SERVER_PORT, path);

        // Get server information
        struct hostent* server = gethostbyname(hostname);    // Lookup IP address of host using DNS
        if(server == NULL) {
            perror("Unable to retrieve server information\n");
            close(proxy_connection_socket);
            continue;
        }

        // printf("SERVER PORT: %d\n", SERVER_PORT);
        // printf("hostname: %s\n", hostname);
        // printf("path: %s\n", path);

        // Create socket for server
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(server_socket  < 0){
            perror("Error creating connection socket");
            exit(EXIT_FAILURE);
        }

        // Create struct for server address
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr)); // Set structure to 0's, ensuring sin_zero is all zeros
        server_addr.sin_family = AF_INET;             // Set address family to IPv4
        server_addr.sin_port = htons(SERVER_PORT);    // Set server port to port associated with web servers (80)
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

        // Connect to server
        if(connect(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            perror("Unable to connect to server");
            close(proxy_connection_socket);
            close(server_socket);
            continue;
        }

        // Send client's HTTP request to server
        snprintf(buffer, BUFFER_SIZE, "GET %s %s\r\nHost: %s\r\nConnection: close\r\n\r\n", path, version, hostname);
        write(server_socket, buffer, strlen(buffer));

        // Read response from web server back into buffer (to be written
        // back to the client)
        while((bytes_read = read(server_socket, buffer, BUFFER_SIZE)) > 0) {
            write(proxy_connection_socket, buffer, BUFFER_SIZE);
        }

        // Close connection with both client and server
        close(proxy_connection_socket);
        close(server_socket);
    }
    close(proxy_listening_socket);

    return 0;
}