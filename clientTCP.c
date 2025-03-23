#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "TCPheaderCode.h"

void parseIpPort(const char *input, char *ip, int *port);

void sendFile(int socket, const char *fileName, int bufSize);

int main(int argc, char *argv[]) {

    checkArgv(argc, argv);

    char *fileName = argv[1];
    char ip[INET_ADDRSTRLEN + 1];
    int givenDestPortNum, mySocket;
	struct sockaddr_in dest; // socket info about the machine we are conencting to
	struct sockaddr_in src; // socket info about our machine
    int bufSize = (argc == 4) ? atoi(argv[3]) : DEFAULT_BUFSIZE; //Set bufSize to passed in argument if given, otherwise use const of 4096

    parseIpPort(argv[2], ip, &givenDestPortNum);

    //Create a socket with error handling
    if ((mySocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: failed to create socket");
        exit(1);
    }

    //Set both structs to be zero'd out
	memset(&dest, 0, sizeof(dest));           
	memset(&src, 0, sizeof(src));       

    //Initialize the destination socket information
	dest.sin_family = AF_INET;	// Use the IPv4 address family
    if (inet_pton(AF_INET, ip, &dest.sin_addr) <= 0) {  // Convert IP address string
        perror("inet_pton failure: Invalid IP address");
        close(mySocket);
        exit(1);
    }
	dest.sin_port = htons(givenDestPortNum); // Set destination port number

    /*
        CONNECTING TO THE SERVER!
    */

    if(connect(mySocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in)) > 0 ){
        perror("Failed to connect to the server");        
        close(mySocket);
        exit(1);
    };
	socklen_t sLen = sizeof(src);
	getsockname(mySocket, (struct sockaddr *) &src, &sLen);

	printf("Outgoing connection from %s on port %d\n", inet_ntoa(src.sin_addr), ntohs(src.sin_port));
  	printf("Outgoing connection to %s on port %d\n", inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
	
    sendFile(mySocket, fileName, bufSize);

    close(mySocket);
    return 0;
}

//Check passed in arguments to ensure they are formatted properly.
void checkArgv(int numArgs, char *givenArgs[]){
    if(numArgs < 3 || numArgs > 4){
        printf("Invalid number of arguments passed in\n");
        printf("USEAGE: ./clientTCP fileName IP:Port BufferSize\n");
        printf("fileName is name of a txt file, IP:Port is an IPv4 address of form 1.1.1.1:55555, and bufferSize is an option int value\n");
        printf("Ensure the arguments are in the correct order. To test locally, pass in localhost as 127.0.0.1 for the IP and give a port number\n");
        exit(1);
    }

    //Check to see if fileName is an actual file
    if(access(givenArgs[1], F_OK) != 0){
        fprintf(stderr, "Unable to access file with name %s. Exiting...\n", givenArgs[1]);
        exit(1);
    }

    //Ensure the bufSize given is a positive int
    if (numArgs == 4 && atoi(givenArgs[3]) <= 0) {
        fprintf(stderr, "Invalid buffer size. It should be a positive integer.\n");
        exit(1);
    }
}

void parseIpPort(const char *input, char *ip, int *port) {
    char *colon = strchr(input, ':');
    if (colon == NULL) {
        fprintf(stderr, "Invalid format for IP and port. Expected IP:Port\n");
        exit(1);
    }

    // Split the IP and port by replacing ':' with '\0'
    *colon = '\0'; 
    strcpy(ip, input);
    *port = atoi(colon + 1);
}

//Sends a file to server on listening socket. 
void sendFile(int socket, const char *fileName, int bufSize){
    size_t fileNameLen = strlen(fileName) + 1; 
    size_t bytesRead;
    FILE *file = fopen(fileName, "rb");
    long fileSize = 0.0;

    if(file == NULL){
        perror("Failed to open the file for transfer");
        return;
    }

    //The client will first send filename length, then filename, file size and then its contents
    //Filename + filename size
    if (send(socket, &fileNameLen, sizeof(fileNameLen), 0) < 0) {
        perror("Error sending filename length");
        fclose(file);
        return;
    }
    if (send(socket, fileName, fileNameLen, 0) < 0) {
        perror("Error sending filename");
        fclose(file);
        return;
    }
    
    //Getting and sending file size
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (send(socket, &fileSize, sizeof(fileSize), 0) < 0) {
        perror("Error sending file size");
        fclose(file);
        return;
    }

    //Setup and sending the file contents
    char *buffer = malloc(bufSize);
    if (buffer == NULL) {
        perror("Error allocating buffer");
        fclose(file);
        return;
    }
    
    while ((bytesRead = fread(buffer, 1, bufSize, file)) > 0) {
        if (send(socket, buffer, bytesRead, 0) < 0) {
            perror("Error sending file data");
            free(buffer);
            fclose(file);
            return;
        }
        printf("Sent %zu bytes\n", bytesRead);
    }

    printf("File transfer successful. Cleaning up resources...\n");
    
    free(buffer);
    fclose(file);
    return;
}
