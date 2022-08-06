/**
 *
 * NAME:		Guanhong Li
 *
 * HOMEWORK:		3
 *
 * CLASS:		ICS 451
 * 
 * INSTRUCTOR:		Ravi Narayan
 *
 * DATE:		9/23/2021
 *
 * FILE:		server.c
 *
 * DESCRIPTION:		This file contains the server for homework 3.
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Starts a simple server which sends a text string upon client connection.
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
int main(int argc, char* argv[]) {

    FILE* filePointer = fopen("output_server.txt", "w");
    char message[] = "Hello Guanhong. ICS 451 is fun!\n";
    int portNumber = atoi(argv[1]);

    // create server TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    fputs("Server socket created.\n", filePointer);
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
    fprintf(filePointer, "Server socket bound at address: %s and port: %d.\n", address, portNumber);
    printf("Server socket bound at address: %s and port: %d.\n", address, portNumber);

    // listen for client connection
    listen(serverSocket, 1);
    fputs("Listening for client connection...\n", filePointer);
    printf("Listening for client connection...\n");

    while (1) {
	// create a new server socket for the client
	int clientSocket = accept(serverSocket, NULL, NULL);
        fputs("Client connected.\n", filePointer);
        printf("Client connected.\n");

	// send the message to the client
	fputs("Sending message to the client...\n", filePointer);
	printf("Sending message to the client...\n");
	send(clientSocket, message, sizeof(message), 0);
	fputs("Message sent to the client.\n", filePointer);
	printf("Message sent to the client.\n");

	// close the client connection socket
	close(clientSocket);
	fputs("Closed client connection.\n", filePointer);
	printf("Closed client connection.\n");

	fflush(filePointer);
    }

    fclose(filePointer);

    return 0;
}
