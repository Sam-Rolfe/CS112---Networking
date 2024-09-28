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
#include <stdbool.h>

#include "cache.h"       
#include "cache_entry.h" 

// ----GLOBAL VARIABLES----------------------------------------------------------------------------
#define DEFAULT_SERVER_PORT 80
#define BUFFER_MAX_SIZE 1000 //10*1024*1024  // Confirm size
#define HTTP_HEADER_MAX_SIZE 1000

//----FUNCTIONS------------------------------------------------------------------------------------
// Check whether request is cached and fresh.
// If so, return response from cache. Else, 
// retrive from server, cache, and return response
void proxy_request(int server_socket, Cache* cache, char* url, char* buffer, int* bytes_read) {
    // TODO: Check whether request is already present (and fresh)
    //       in cache
    bool cache_hit = false;
    
    // TODO: Get response from server, pass back to client, 
    //       and add to cache
    // write(server_socket, buffer, strlen(buffer));
    *bytes_read = read(server_socket, buffer, sizeof(buffer));
}

//----MAIN-----------------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    printf("Start\n");
    // Declare variables
    int PROXY_PORT;
    int proxy_listening_socket, client_socket;
    struct sockaddr_in proxy_addr, client_addr; // Struct for handling internet addresses
    socklen_t client_addr_size = sizeof(client_addr);
    char buffer[BUFFER_MAX_SIZE];
    Cache* cache = create_cache();

    // Get port number from argv
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }
    PROXY_PORT = atoi(argv[1]);

    // Create listening socket
    proxy_listening_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(proxy_listening_socket < 0) {
        perror("Error creating listening socket");
        return -1;
    }

    // Initialize fields of struct for proxy server address
    memset(&proxy_addr, 0, sizeof(proxy_addr)); // Set structure to 0's, ensuring sin_zero is all zeros
    proxy_addr.sin_family = AF_INET;            // Set address family to IPv4
    proxy_addr.sin_addr.s_addr = INADDR_ANY;    // Set IP address to all IP addresses of machine
    proxy_addr.sin_port = htons(PROXY_PORT);    // Set port number

    // Set socket options to allow reuse of the address (fixes "address already in use" bug)
    int opt = 1;
    if (setsockopt(proxy_listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Error setting socket options");
        close(proxy_listening_socket);
        return -1;
    }   

    // Bind socket to IP address and port (specificied in proxy_addr)
    if(bind(proxy_listening_socket, (struct sockaddr *) &proxy_addr, sizeof(proxy_addr)) < 0 ){
        perror("Error binding socket");
        return -1;
    }

    // Listen for incoming connections requests on "listening socket"
    listen(proxy_listening_socket, 5);
    printf("Listening for incoming connection requests on port %d...\n\n", PROXY_PORT);

    while(1) {
        // Yield CPU and await connection request. Upon reception,
        // bind to dedicated socket
        client_socket = accept(proxy_listening_socket, (struct sockaddr *) &client_addr, &client_addr_size);
        if(client_socket  < 0){
            perror("Error creating connection socket");
            return -1;
        }
    
        // Read contents from client connection into buffer
        memset(buffer, 0, BUFFER_MAX_SIZE);
        int bytes_read = read(client_socket, buffer, BUFFER_MAX_SIZE - 1);
        if(bytes_read < 0) {
            perror("Error reading from connection socket");
            return -1;
        }

        // printf("Buffer received from client: \n%s", buffer);

        // Parse contents of client request into HTTP request components
        char method[HTTP_HEADER_MAX_SIZE], url[HTTP_HEADER_MAX_SIZE], version[HTTP_HEADER_MAX_SIZE];
        sscanf(buffer, "%s %s %s", method, url, version);

        // Confirm request is GET request (as per project specs)
        if (strncmp(method, "GET", 3) != 0) {
            perror("Proxy server only accepts 'GET' requests");
            close(client_socket);
            continue;
        }

        // Get the hostname, path, and port number (if present) from URL
        int SERVER_PORT = DEFAULT_SERVER_PORT;
        char hostname[HTTP_HEADER_MAX_SIZE], path[HTTP_HEADER_MAX_SIZE];
        int n_objects_assigned = sscanf(url, "http://%[^:/]%*[:]%d%[^\n]", hostname, &SERVER_PORT, path);
        // If parsing failed to assign 3 objects, port is not present in URL.
        // Parse again, this time excluding port
        if(n_objects_assigned < 3) {
            sscanf(url, "http://%[^/]%[^\n]", hostname, path);
        }
        // printf("n_objects_assigned: %d", n_objects_assigned);
        // printf("\nhostname: %s\nSERVER_PORT: %d\npath: %s\n\n", hostname, SERVER_PORT, path);

        // Get server information
        struct hostent* server = gethostbyname(hostname);    // Lookup IP address of host using DNS
        if(server == NULL) {
            perror("Unable to retrieve server information\n");
            close(client_socket);
            continue;
        }

        // Create socket for server
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(server_socket  < 0){
            perror("Error creating connection socket");
            return -1;
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
            close(client_socket);
            close(server_socket);
            continue;
        }

        // Send client's HTTP request to server
        write(server_socket, buffer, strlen(buffer));

        // proxy_receive(server_socket);
        while((bytes_read = read(server_socket, buffer, sizeof(buffer))) > 0) {
            write(client_socket, buffer, bytes_read);
        }
        // proxy_request(server_socket, cache, url, buffer, &bytes_read);

        // write(client_socket, buffer, bytes_read);
        printf("\n\n");

        // Close connection with both client and server
        close(client_socket);
        close(server_socket);
    }
    close(proxy_listening_socket);

    return 0;
}

//-------------------------------------------------------------------------------------------------