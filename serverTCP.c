//#define _XOPEN_SOURCE 700 //If this isn't here, everything complains about signaler being an incomplete struct of sigaction.
//"prevent nonstandard definitions from being exposed" or "expose nonstandard definitions that are not exposed by default". I guess??

#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "TCPheaderCode.h"

int mySocket;            // socket used to listen for incoming connections
int consocket;           // current socket we are accepting from(?)

void receiveFile(int socket, int bufSize, const char* clientIP);

char* generateUniqueFileName(char* originalName);

int main(int argc, char *argv[]){
    
    checkArgv(argc, argv);

    double start = 0, stop = 0;
    int flag = 1;
    int servPortNum = atoi(argv[1]);
    int bufSize = (argc == 3) ? atoi(argv[2]) : DEFAULT_BUFSIZE;
	struct sockaddr_in dest; // socket info about the machine connecting to us
	struct sockaddr_in serv; // socket info about our server

    //Set up socket info
	socklen_t socksize = sizeof(struct sockaddr_in);
	memset(&serv, 0, sizeof(serv));           // zero the struct before filling the fields
	serv.sin_family = AF_INET;                // Use the IPv4 address family
	serv.sin_addr.s_addr = htonl(INADDR_ANY); // Set our address to any interface 
	serv.sin_port = htons(servPortNum);       // Set the server port number 

    //Create the socket
    if ((mySocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("client: failed to create socket");
        exit(1);
    }

    //Set the socket options
	if (setsockopt(mySocket,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag)) == -1) {
    	printf("setsockopt() failed\n");
		printf("%s\n", strerror(errno));
    	exit(1);
	} 

    // bind serv information to mysocket
	if (bind(mySocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) != 0){
		printf("Unable to open TCP socket on localhost:%d\n", servPortNum);

		printf("%s\n", strerror(errno));
		close(mySocket);
		return 0;
	}

    //start listening, allowing a queue of up to MAX_QUEUE_LEN pending connection
    printf("Listening for client connection\n");
    fflush(stdout);
	if(listen(mySocket, MAX_QUEUE_LEN) < 0){
        fprintf(stderr, "Unable to open TCP socket on localhost:%d\n", servPortNum);
		printf("%s\n", strerror(errno));
		close(mySocket);
    };

    // Create a socket to communicate with the client that just connected
	consocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);

    while(consocket) {
		//dest contains information - IP address, port number, etc. - in network byte order
		//We need to conert it to host byte order before displaying it
        printf("Incoming connection from %s on port %d\n", inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
        
        start = clock();
        receiveFile(consocket, bufSize, inet_ntoa(dest.sin_addr));
		stop = clock();

        printf("-------------------------\n");
        printf("Receiving file took %lf\n", (((double)(stop-start))/CLOCKS_PER_SEC));
        printf("-------------------------\n\n");

        fflush(stdout);
		//Close current connection
		close(consocket);
		
		//Continue listening for incoming connections
		consocket = accept(mySocket, (struct sockaddr *)&dest, &socksize);
	}

	close(mySocket);
	return 0;
}

//Check passed in arguments to ensure they are formatted properly.
void checkArgv(int numArgs, char *givenArgs[]){
    if(numArgs < 2 || numArgs > 3){
        printf("Invalid number of arguments passed in\n");
        printf("USEAGE: ./serverTCP portNum bufSize\n");
        printf("portNum is the port the server will be listening on and bufferSize is an option int value for size of buffer\n");
        printf("Ensure the arguments are in the correct order.\n");
        exit(0);
    }

    //Ensure the bufSize given is a positive int
    if (numArgs == 3 && atoi(givenArgs[2]) <= 0) {
        fprintf(stderr, "Invalid buffer size. It should be a positive integer.\n");
        exit(0);
    }
}

void receiveFile(int socket, int bufSize, const char* clientIP) {
    size_t fileNameLen;
    ssize_t bytesReceived;
    char* fileName;
    char* buffer = malloc(bufSize);
    long fileSize = 0.0;
    long totalReceived = 0;

    //Receive filename length
    if (recv(socket, &fileNameLen, sizeof(fileNameLen), 0) <= 0) {
        perror("Error receiving filename length");
        return;
    }

    //Receive filename
    fileName = malloc(fileNameLen);
    if (fileName == NULL) {
        perror("Memory allocation failed");
        return;
    }
    if (recv(socket, fileName, fileNameLen, 0) <= 0) {
        perror("Error receiving filename");
        free(fileName);
        return;
    }

    //Receive file size
    if (recv(socket, &fileSize, sizeof(fileSize), 0) <= 0) {
        perror("Error receiving file size");
        free(fileName);
        return;
    }

    //Generate unique filename if needed
    char* uniqueFileName = generateUniqueFileName(fileName);

    //Create output file
    FILE *file = fopen(uniqueFileName, "wb");
    if (file == NULL) {
        perror("Error creating output file");
        free(uniqueFileName);
        return;
    }

    // Display file information
    printf("\nReceiving file:\n");
    printf("- File name: %s\n", fileName);
    printf("- Total size: %ld bytes\n", fileSize);
    printf("- Source IP: %s\n", clientIP);
    printf("- Server buffer size: %d bytes\n\n", bufSize);
    fflush(stdout);

    //Allocate receive buffer
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        free(uniqueFileName);
        return;
    }

    //Receive file contents, error check, and finally write to file
    while (totalReceived < fileSize) {
        bytesReceived = recv(socket, buffer, 
                           (fileSize - totalReceived < bufSize) ? 
                           (fileSize - totalReceived) : bufSize, 0);
        
        if (bytesReceived <= 0) {
            printf("BytesRecieved <= 0. Done reading from %s\n\n", fileName);
            fflush(stdout);
            break;
        }

        if (fwrite(buffer, 1, bytesReceived, file) != bytesReceived) {
            perror("Error writing to file");
            break;
        }
    }


    fclose(file);
    free(fileName); 
    free(buffer);
    free(uniqueFileName);
    
    return;
}

char* generateUniqueFileName(char* originalName){
    //char* baseName = strcpy(baseName, originalName);
    char* extension = strrchr(originalName, '.');
    char* uniqueFileName;
    int counter = 1;

    if (extension) {
        *extension = '\0';
    }
    
    uniqueFileName = malloc(strlen(originalName) + 20);
    
    sprintf(uniqueFileName, "%s.txt", originalName);
    
    //Check if access returns 0 (if it does we have a duplicate name. Add numbers to file until its unique)
    while (access(uniqueFileName, F_OK) == 0) {
        sprintf(uniqueFileName, "%s(%d).txt", originalName, counter);
        counter++;
    }
    
    return uniqueFileName;
}

