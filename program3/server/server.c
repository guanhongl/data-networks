/**
 *
 * NAME:		Guanhong Li
 *
 * HOMEWORK:		4
 *
 * CLASS:		ICS 451
 * 
 * INSTRUCTOR:		Ravi Narayan
 *
 * DATE:		10/6/2021
 *
 * FILE:		server.c
 *
 * DESCRIPTION:		This file contains the server for homework 4.
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Starts a simple server which sends a .jpeg file upon client connection.
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
int main(int argc, char* argv[]) {

    FILE* filePointer = fopen("file_server.jpg", "rb"); // open as a binary file
    int fileSize = 49569;
    int portNumber = atoi(argv[1]);

    // create server TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("Server socket created.\n");

    // see https://man7.org/linux/man-pages/man7/ip.7.html
    struct sockaddr_in serverAddress = {
	AF_INET,		// address family: AF_INET
	htons(portNumber),	// port in network byte order; htons converts host byte order to network byte order
	INADDR_ANY		// internet address; INADDR_ANY refers to any address for binding
    };
    
    // bind server socket to address and port
    bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    char address[256];
    inet_ntop(AF_INET, &(serverAddress.sin_addr), address, sizeof(address)); // inet_ntop converts network address to a string
    printf("Server socket bound at address: %s and port: %d.\n", address, portNumber);

    // listen for client connection
    listen(serverSocket, 1);
    printf("Listening for client connection...\n");

    while (1) {
	// create a new server socket for the client
	int clientSocket = accept(serverSocket, NULL, NULL);
        printf("Client connected.\n");

	// send the .jpeg file to the client
	printf("Sending .jpeg file to the client...\n");
	int status = sendfile(clientSocket, fileno(filePointer), NULL, fileSize);
	if (status == fileSize) {
	    printf(".jpeg file sent to the client.\n");
	}
	else {
	    printf("Error: failed to send .jpeg file to the client.\n");
	}

	rewind(filePointer); // set position of filePointer to the beginning
    }

    return 0;
}
