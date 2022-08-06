/**
 *
 * NAME:		Guanhong Li
 *
 * HOMEWORK:	8
 *
 * CLASS:		ICS 451
 * 
 * INSTRUCTOR:	Ravi Narayan
 *
 * DATE:		11/16/2021
 *
 * FILE:		server.c
 *
 * DESCRIPTION:	This file contains the server for homework 8.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

/**
 * Initializes the sequence number header with a random initial sequence number
 * (initializes a byte at a time because RAND_MAX is only guaranteed to be 32767).
 * Returns the sequence number.
 */
unsigned int initSeqNum(unsigned char* serverHeader) {

    unsigned int seqNum = 0;
    srand(getpid());
    
    for (int i = 0; i < 4; i++) {
        unsigned char randByte = rand() & 0xFF; // get a random byte in range 0x00 to 0xFF
        serverHeader[i + 4] = randByte;
        seqNum |= randByte << (24 - i * 8); // build the sequence number starting w/ the leftmost byte
    }

    return seqNum;
}

/**
 * Initializes the header fields for the SYNACK segment.
 * Returns the sequence number.
 */
unsigned int initSynAck(unsigned char* serverHeader, unsigned char* syn) {

    // dest port number; get from syn src port number
    serverHeader[2] = syn[0];
    serverHeader[3] = syn[1];
    // sequence number
    unsigned int seqNum = initSeqNum(serverHeader);
    // ack number; syn seq num + 1
    unsigned int ackNum = (syn[4] << 24) | (syn[5] << 16) | (syn[6] << 8) | syn[7]; // build the SYN seqNum
    ackNum++;
    // for each byte in ackNum:
    for (int i = 0; i < 4; i++) {
        serverHeader[i + 8] = ackNum >> (24 - i * 8); // write the ack number starting w/ the leftmost byte
    }
    // right 2 bits of reserved data + flags
    serverHeader[13] = 0x12; // 0x12 = b0001 0010; ACK, SYN set

    return seqNum;
}

/**
 * Pretty print the header.
 */
void prettyPrint(unsigned char* header, FILE* fp) {

    unsigned int srcPort = (header[0] << 8) | header[1];
    unsigned int destPort = (header[2] << 8) | header[3];
    unsigned int seqNum = (header[4] << 24) | (header[5] << 16) | (header[6] << 8) | header[7];
    unsigned int ackNum = (header[8] << 24) | (header[9] << 16) | (header[10] << 8) | header[11];
    unsigned int winSize = (header[14] << 8) | header[15];

    for (int i = 0; i < 20; i ++) {
        printf("%02X ", header[i]);
        fprintf(fp, "%02X ", header[i]);
    }
    printf("\n");
    fprintf(fp, "\n");
    printf("-------------------------------------------------\n");
    fprintf(fp, "-------------------------------------------------\n");
    printf("Header\t\t\tByte(s)\t\tValue\n");
    fprintf(fp, "Header\t\t\tByte(s)\t\tValue\n");
    printf("-------------------------------------------------\n");
    fprintf(fp, "-------------------------------------------------\n");
    printf("Source Port:\t\t%02X %02X\t\t%u\n", header[0], header[1], srcPort);
    fprintf(fp, "Source Port:\t\t%02X %02X\t\t%u\n", header[0], header[1], srcPort);
    printf("Destination Port:\t%02X %02X\t\t%u\n", header[2], header[3], destPort);
    fprintf(fp, "Destination Port:\t%02X %02X\t\t%u\n", header[2], header[3], destPort);
    printf("Sequence Number:\t%02X %02X %02X %02X\t%u\n", header[4], header[5], header[6], header[7], seqNum);
    fprintf(fp, "Sequence Number:\t%02X %02X %02X %02X\t%u\n", header[4], header[5], header[6], header[7], seqNum);
    printf("Acknowledgement Number:\t%02X %02X %02X %02X\t%u\n", header[8], header[9], header[10], header[11], ackNum);
    fprintf(fp, "Acknowledgement Number:\t%02X %02X %02X %02X\t%u\n", header[8], header[9], header[10], header[11], ackNum);
    printf("Data Offset:\t\t%X\t\t%u\n", header[12] >> 4, 0);
    fprintf(fp, "Data Offset:\t\t%X\t\t%u\n", header[12] >> 4, 0);
    printf("Reserved Data\t\t\t\t%u\n", 0); // reserved data = 0
    fprintf(fp, "Reserved Data\t\t\t\t%u\n", 0);
    printf("And Control Flags:\t%X %02X\t\tSYN, ACK: Set\n", header[12] & 0x0F, header[13]);
    fprintf(fp, "And Control Flags:\t%X %02X\t\tSYN, ACK: Set\n", header[12] & 0x0F, header[13]);
    printf("Window Size:\t\t%02X %02X\t\t%u\n", header[14], header[15], winSize);
    fprintf(fp, "Window Size:\t\t%02X %02X\t\t%u\n", header[14], header[15], winSize);
    printf("TCP Checksum:\t\t%02X %02X\n", header[16], header[17]);
    fprintf(fp, "TCP Checksum:\t\t%02X %02X\n", header[16], header[17]);
    printf("Urgent Pointer\t\t%02X %02X\t\t%u\n", header[18], header[19], 0);
    fprintf(fp, "Urgent Pointer\t\t%02X %02X\t\t%u\n", header[18], header[19], 0);
    printf("-------------------------------------------------\n\n");
    fprintf(fp, "-------------------------------------------------\n\n");
}

/**
 * Starts a simple TCP server which simulates a three way handshake upon client connection.
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
int main(int argc, char* argv[]) {

    FILE* filePointer = fopen("output_server.txt", "w");
    unsigned int srcPort = atoi(argv[1]);

    // create server TCP socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("Server socket created.\n");
    fprintf(filePointer, "Server socket created.\n");

    // see https://man7.org/linux/man-pages/man7/ip.7.html
    struct sockaddr_in serverAddress = {
        AF_INET,		    // address family: AF_INET
        htons(srcPort),	    // port in network byte order; htons converts host byte order to network byte order
        INADDR_ANY		    // internet address; INADDR_ANY refers to any address for binding
    };
    
    // bind server socket to address and port
    bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    char address[256];
    inet_ntop(AF_INET, &(serverAddress.sin_addr), address, sizeof(address)); // inet_ntop converts network address to a string
    printf("Server socket bound at address: %s and port: %d.\n", address, srcPort);
    fprintf(filePointer, "Server socket bound at address: %s and port: %d.\n", address, srcPort);

    // listen for client connection
    int listenStatus = listen(serverSocket, 1);

    if (listenStatus == 0) {
        printf("Listening for client connection...\n");
        fprintf(filePointer, "Listening for client connection...\n");

        // TCP server header
        unsigned char serverHeader[20];
        // source port number
        serverHeader[0] = srcPort >> 8; // get the left byte
        serverHeader[1] = srcPort;
        // data offset (b0000) + left 4 bits of reserved data (b0000)
        serverHeader[12] = 0x00;
        // window size
        serverHeader[14] = 0x44;
        serverHeader[15] = 0x70;
        // checksum
        serverHeader[16] = 0xFF;
        serverHeader[17] = 0xFF;
        // urgent pointer
        serverHeader[18] = 0x00;
        serverHeader[19] = 0x00;

        while (1) {
            // create a new server socket for the client
            int clientSocket = accept(serverSocket, NULL, NULL);
            printf("Client connected.\n\n");
            fprintf(filePointer, "Client connected.\n\n");

            // receive SYN segment
            unsigned char syn[20];
            int synStatus = recv(clientSocket, &syn, sizeof(syn), 0);

            // if SYN received and SYN flag set:
            if (synStatus == sizeof(syn) && syn[13] == 0x02) {
                printf("SYN segment received from the client.\n\n");
                fprintf(filePointer, "SYN segment received from the client.\n\n");

                unsigned int seqNum = initSynAck(serverHeader, syn);
                // send SYNACK segment
                send(clientSocket, serverHeader, sizeof(serverHeader), 0);
                printf("SYNACK segment sent to the client:\n");
                fprintf(filePointer, "SYNACK segment sent to the client:\n");
                prettyPrint(serverHeader, filePointer);

                // increment sequence number by 1
                seqNum++;
                for (int i = 0; i < 4; i++) {
                    serverHeader[i + 4] = seqNum >> (24 - i * 8); // write the sequence number, starting w/ the leftmost byte
                }

                // receive ACK segment
                unsigned char ack[20];
                int ackStatus = recv(clientSocket, &ack, sizeof(ack), 0);
                printf("Awaiting ACK segment...\n");
                fprintf(filePointer, "Awaiting ACK segment...\n");

                // if ACK received and ACK flag set and seqNum == ackNum:
                if (ackStatus == sizeof(ack) && ack[13] == 0x10 && serverHeader[4] == ack[8] &&
                    serverHeader[5] == ack[9] && serverHeader[6] == ack[10] && serverHeader[7] == ack[11]) {
                    printf("ACK segment received from the client.\n\n");
                    fprintf(filePointer, "ACK segment received from the client.\n\n");

                    printf("TCP connection established.\n");
                    fprintf(filePointer, "TCP connection established.\n");
                }
                else {
                    perror("Error receiving ACK");
                }
            }
            else {
                perror("Error receiving SYN");
            }

            // wait for client disconnect, then close the client socket
            unsigned char* zero;
            int zeroStatus = recv(clientSocket, &zero, 0, 0); // returns 0 on client disconnect
            if (zeroStatus == 0) {
                printf("Client disconnected.\n");
                fprintf(filePointer, "Client disconnected.\n");
                close(clientSocket);
            }

            fflush(filePointer);
        }
    }
    else {
        perror("listen() failed");
    }

    // leave the server socket open...

    return 0;
}
