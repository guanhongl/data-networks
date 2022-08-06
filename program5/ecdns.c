/**
 *
 * NAME:		Guanhong Li
 *
 * HOMEWORK:	EC
 *
 * CLASS:		ICS 451
 * 
 * INSTRUCTOR:	Ravi Narayan
 *
 * DATE:		12/9/2021
 *
 * FILE:		ecdns.c
 *
 * DESCRIPTION:	This file contains the server for the EC assignment.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <search.h>
#include <string.h>

/**
 * Gets the hostname from the query
 * @param query the query
 * @param domainName the domain name buffer to write to
 * @return the hostname length
 */
int getHostname(unsigned char* query, char* domainName) {

    unsigned char length;
    int j, k, hostnameLen;

    length = query[12]; // the first byte of question
    j = 13; // the byte index
    k = 0;  // the name index
    hostnameLen = 0;

    while (length != 0x00) {

        hostnameLen += (length + 1);

        if (k != 0) {
            domainName[k] = '.';
            k++;
        }

        for (unsigned char i = 0; i < length; i++) {
            domainName[k] = (char) query[j];
            j++;
            k++;
        }
        
        length = query[j];
        j++;
    }

    hostnameLen++;
    domainName[k] = '\0';

    return hostnameLen;
}

/**
 * Converts the IP address (string) to byte order
 * @param string the IP address
 * @param ipAddress the IP address buffer to write to
 */
void getIPAddress(char* string, unsigned char* ipAddress) {
    // copy the ip address (for strtok)
    char copy[strlen(string) + 1];
    strcpy(copy, string);

    // convert the string to hex
    unsigned int segment;
    int i = 0;
    char* token = strtok(copy, ".");

    while (token != NULL) {

        segment = atoi(token);
        ipAddress[i] = (unsigned char) segment;

        token = strtok(NULL, ".");
        i++;
    }
}

/**
 * Constructs the DNS response
 * @param query the query
 * @param ipAddress the IP address
 * @param response the response buffer to write to
 * @param hostnameLen the hostname length
 */
void getDNSResponse(unsigned char* query, unsigned char* ipAddress, unsigned char* response, int hostnameLen) {
    // response to query ID
    response[0] = query[0];
    response[1] = query[1];
    // flags
    response[2] = 0x81; // QR set, RD set
    response[3] = 0x80; // RA set
    // QDCOUNT (1)
    response[4] = 0x00;
    response[5] = 0x01;
    // ANCOUNT (1)
    response[6] = 0x00;
    response[7] = 0x01;
    // NSCOUNT (0)
    response[8] = 0x00;
    response[9] = 0x00;
    // ARCOUNT (0)
    response[10] = 0x00;
    response[11] = 0x00;

    // question section (copied from query)
    int i = 12;
    for (i; i < (hostnameLen + 12 + 4); i++) {
        response[i] = query[i];
    }

    // response section:
    // pointer to the name
    response[i] = 0xc0;
    response[i + 1] = 0x0c;
    // TYPE (A)
    response[i + 2] = 0x00;
    response[i + 3] = 0x01;
    // CLASS (IN)
    response[i + 4] = 0x00;
    response[i + 5] = 0x01;
    // TTL (600s)
    response[i + 6] = 0x00;
    response[i + 7] = 0x00;
    response[i + 8] = 0x02;
    response[i + 9] = 0x58;
    // RDLENGTH (4 bytes)
    response[i + 10] = 0x00;
    response[i + 11] = 0x04;
    // RDATA
    response[i + 12] = ipAddress[0];
    response[i + 13] = ipAddress[1];
    response[i + 14] = ipAddress[2];
    response[i + 15] = ipAddress[3];
}

/**
 * Starts a simple DNS server which answers requests for names it is authoritative for.
 * These are specified on the command line as domain.name/ip.address.
 * @param argc the number of command line arguments
 * @param argv the command line arguments
 */
int main(int argc, char* argv[]) {

    char* entryName;
    char* entryAddress;
    unsigned int portNumber = atoi(argv[1]);
    // see https://man7.org/linux/man-pages/man3/hcreate.3.html
    ENTRY entry;
    ENTRY *entryPointer;
    hcreate(50); // create a hash table with MAX 50 entries
    
    // for each domain.name/ip.address:
    for (int i = 2; i < argc; i++) {

        entryName = strtok(argv[i], "/");
        entryAddress = strtok(NULL, " ");

        entry.key = entryName;
        entry.data = entryAddress;
        hsearch(entry, ENTER); // insert the key, data mapping
    }
    
    // create server UDP socket
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    printf("Server socket created.\n");

    // see https://man7.org/linux/man-pages/man7/ip.7.html
    struct sockaddr_in serverAddress = {
        AF_INET,		    // address family: AF_INET
        htons(portNumber),	// port in network byte order; htons converts host byte order to network byte order
        INADDR_ANY		    // internet address; INADDR_ANY refers to any address for binding
    };
    
    // bind server socket to address and port
    bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    char address[256];
    inet_ntop(AF_INET, &(serverAddress.sin_addr), address, sizeof(address)); // inet_ntop converts network address to a string
    printf("Server socket bound at address: %s and port: %d.\n", address, portNumber);

    struct sockaddr_in dest; // the dest socket
    socklen_t destSize = sizeof(dest);
    unsigned char query[65536]; // the query (bytes)
    char domainName[256]; // the domain name (string)
    int hostnameLen; // the hostname length in query
    unsigned char response[512]; // the response (bytes)
    unsigned char ipAddress[4]; // the ip address (bytes)

    while (1) {
        // receive the DNS query
        recvfrom(serverSocket, &query, sizeof(query), 0, (struct sockaddr *) &dest, &destSize);
        printf("DNS query received from the client.\n");

        // if query is a Type A query
        hostnameLen = getHostname(query, domainName);

        // prepare the DNS response:

        // find the corresponding ip address in the hash table
        entry.key = domainName;
        entryPointer = hsearch(entry, FIND);
        
        // if server is not authoritative for the domain name:
        if (entryPointer == NULL) {
            printf("Error: No such entry\n");
        }
        else {
            getIPAddress((char*) entryPointer->data, ipAddress);
            
            getDNSResponse(query, ipAddress, response, hostnameLen);

            // send the DNS response to the client
            printf("Sending DNS response to the client...\n");
            sendto(serverSocket, response, sizeof(response), 0, (struct sockaddr *) &dest, destSize);
            printf("DNS response sent to the client.\n");
        }
    }

    // leave the server socket open...

    hdestroy();
    
    return 0;
}
