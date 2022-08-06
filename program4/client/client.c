/**
 *
 * NAME:		Guanhong Li
 *
 * HOMEWORK:    8
 *
 * CLASS:		ICS 451
 * 
 * INSTRUCTOR:	Ravi Narayan
 *
 * DATE:		11/16/2021
 *
 * FILE:		client.c
 *
 * DESCRIPTION:	This file contains the client for homework 8.
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
unsigned int initSeqNum(unsigned char* clientHeader) {

    unsigned int seqNum = 0;
    srand(getpid());
    
    for (int i = 0; i < 4; i++) {
        unsigned char randByte = rand() & 0xFF; // get a random byte in range 0x00 to 0xFF
        clientHeader[i + 4] = randByte;
        seqNum |= randByte << (24 - i * 8); // build the sequence number starting w/ the leftmost byte
    }

    return seqNum;
}

/**
 * Initializes the header fields for the SYN segment.
 * Returns the sequence number.
 */
unsigned int initSyn(unsigned char* clientHeader) {

    // sequence number
    unsigned int seqNum = initSeqNum(clientHeader);
    // ack number
    unsigned int ackNum = 0;
    for (int i = 8; i < 12; i++) {
        clientHeader[i] = 0x00;
    }
    // right 2 bits of reserved data + flags
    clientHeader[13] = 0x02; // 0x02 = b0000 0010; SYN set

    return seqNum;
}

/**
 * Initializes the header fields for the ACK segement.
 */
void initAck(unsigned char* clientHeader, unsigned char* synAck) {

    // ack number; SYNACK seq num + 1
    unsigned int ackNum = (synAck[4] << 24) | (synAck[5] << 16) | (synAck[6] << 8) | synAck[7]; // build the SYNACK seqNum
    ackNum++;
    // for each byte in ackNum:
    for (int i = 0; i < 4; i++) {
        clientHeader[i + 8] = ackNum >> (24 - i * 8); // write the ack number starting w/ the leftmost byte
    }
    // right 2 bits of reserved data + flags
    clientHeader[13] = 0x10; // 0x10 = b0001 0000; ACK set
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
    printf("And Control Flags:\t%X %02X\t\t", header[12] & 0x0F, header[13]);
    fprintf(fp, "And Control Flags:\t%X %02X\t\t", header[12] & 0x0F, header[13]);
    if (header[13] == 0x02) {
        printf("SYN: Set\n");
        fprintf(fp, "SYN: Set\n");
    }
    else {
        printf("ACK: Set\n");
        fprintf(fp, "ACK: Set\n");
    }
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
 * Starts a simple TCP client which simulates a three way handshake upon server connection.
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
int main(int argc, char* argv[]) {

    FILE* filePointer = fopen("output_client.txt", "w");
    unsigned int destPort = atoi(argv[1]);

    // create client TCP socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("Client socket created.\n");
    fprintf(filePointer, "Client socket created.\n");

    // see https://man7.org/linux/man-pages/man7/ip.7.html
    struct sockaddr_in serverAddress = {
        AF_INET,		    // address family: AF_INET
        htons(destPort),    // port in network byte order; htons converts host byte order to network byte order
        INADDR_ANY		    // internet address; INADDR_ANY refers to any address for binding
    };
    
    // connect the client socket to the server socket
    printf("Initiating connection to the server...\n");
    fprintf(filePointer, "Initiating connection to the server...\n");
    int connectStatus = connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    
    if (connectStatus == 0) {
        printf("Client connected to the server.\n\n");
        fprintf(filePointer, "Client connected to the server.\n\n");

        // TCP client header
        unsigned char clientHeader[20];
        // source port number
        struct sockaddr_in clientAddress;
        socklen_t addressSize = sizeof(clientAddress);
        getsockname(clientSocket, (struct sockaddr *) &clientAddress, &addressSize); // get the port to which clientSocket is bound
        unsigned int srcPort = ntohs(clientAddress.sin_port); // converts network byte order to host byte order
        clientHeader[0] = srcPort >> 8; // get the left byte
        clientHeader[1] = srcPort;
        // dest port number
        clientHeader[2] = destPort >> 8; // get the left byte
        clientHeader[3] = destPort;
        // data offset (b0000) + left 4 bits of reserved data (b0000)
        clientHeader[12] = 0x00;
        // window size
        clientHeader[14] = 0x44;
        clientHeader[15] = 0x70;
        // checksum
        clientHeader[16] = 0xFF;
        clientHeader[17] = 0xFF;
        // urgent pointer
        clientHeader[18] = 0x00;
        clientHeader[19] = 0x00;

        unsigned int seqNum = initSyn(clientHeader);
        // send SYN segment
        send(clientSocket, clientHeader, sizeof(clientHeader), 0);
        printf("SYN segment sent to the server:\n");
        fprintf(filePointer, "SYN segment sent to the server:\n");
        prettyPrint(clientHeader, filePointer);

        // increment sequence number by 1
        seqNum++;
        for (int i = 0; i < 4; i++) {
            clientHeader[i + 4] = seqNum >> (24 - i * 8); // write the sequence number, starting w/ the leftmost byte
        }

        // receive SYNACK segment
        unsigned char synAck[20];
        int synAckStatus = recv(clientSocket, &synAck, sizeof(synAck), 0);
        printf("Awaiting SYNACK segment...\n");
        fprintf(filePointer, "Awaiting SYNACK segment...\n");

        // if SYNACK received and SYN, ACK flags set and seqNum == ackNum:
        if (synAckStatus == sizeof(synAck) && synAck[13] == 0x12 && clientHeader[4] == synAck[8] &&
            clientHeader[5] == synAck[9] && clientHeader[6] == synAck[10] && clientHeader[7] == synAck[11]) { 
            printf("SYNACK segment received from the server.\n\n");
            fprintf(filePointer, "SYNACK segment received from the server.\n\n");

            // send ACK segment
            initAck(clientHeader, synAck);
            send(clientSocket, clientHeader, sizeof(clientHeader), 0);
            printf("ACK segment sent to the server:\n");
            fprintf(filePointer, "ACK segment sent to the server:\n");
            prettyPrint(clientHeader, filePointer);

            printf("TCP connection established.\n");
            fprintf(filePointer, "TCP connection established.\n");
            // increment sequence number by 1...
        }
        else {
            perror("Error receiving SYNACK");
        }            
    }
    else {
        perror("connect() failed");
    }

    // close the client socket
    close(clientSocket);
    printf("Client socket closed.\n");
    fprintf(filePointer, "Client socket closed.\n");
    fclose(filePointer);

    return 0;
}
