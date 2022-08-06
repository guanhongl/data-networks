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
 * FILE:		client.c
 *
 * DESCRIPTION:		This file contains the client for homework 4.
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Starts a simple client which receives a .jpeg file upon server connection.
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
int main(int argc, char* argv[]) {

    FILE* filePointer = fopen("file_client.jpg", "wb"); // open as a binary file
    int fileSize = 49569;
    int portNumber = atoi(argv[1]);

    // create client TCP socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("Client socket created.\n");

    // see https://man7.org/linux/man-pages/man7/ip.7.html
    struct sockaddr_in serverAddress = {
	AF_INET,		// address family: AF_INET
	htons(portNumber),	// port in network byte order; htons converts host byte order to network byte order
	INADDR_ANY		// internet address; INADDR_ANY refers to any address for binding
    };
    
    // connect the client socket to the server socket
    printf("Initiating connection to the server...\n");
    connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    printf("Client connected to the server.\n");

    // receive the .jpeg file from the server
    char response[fileSize];
    printf("Receiving .jpeg file from the server...\n");
    int status = recv(clientSocket, &response, fileSize, 0);
    if (status == fileSize) {
	fwrite(response, fileSize, 1, filePointer); // write in byte format
	printf(".jpeg file received from the server.\n");
    }
    else {
	printf("Error: failed to receive .jpeg file from the server.\n");
    }

    // close the client socket
    close(filePointer);
    close(clientSocket);
    printf("Client socket closed.\n");

    return 0;
}
