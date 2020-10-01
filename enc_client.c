#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define S_TIME 50000 //wait time of 40 milliseconds

void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

char* srMessage(char* pText, char* key, int socketFD) {
	int tLen = strlen(pText);
    int test1; 
    int test2;
	char* eText;  

	// Send size of file to server
	send(socketFD, &tLen, sizeof(int), 0); 
	usleep(S_TIME);

	// Send pText to be encrypted
	send(socketFD, pText, tLen * sizeof(char), 0); 
	usleep(S_TIME);	

	// Send encryption key
	send(socketFD, key, tLen * sizeof(char), 0); // Send key
	usleep(S_TIME);
    
    eText = malloc(tLen * sizeof(char));

	// Receive encrypted message from server and store in eText
	recv(socketFD, eText, tLen * sizeof(char), 0);  		           

    eText[tLen] = '\0'; 

	return eText;
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){
 
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname); 
    if (hostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

char* fileContents(char* fileName) {
	FILE* file = fopen(fileName, "r");	// Open file with matching filename
    if (file == NULL) {	// Error opening the file
		fprintf(stderr, "Error opening file: %s\n", fileName);
		exit(1);
	}
	char* line;	// holds the info from the file
    
    size_t pos = ftell(file);    // Current position
    fseek(file, 0, SEEK_END);    // Go to end
    size_t length = ftell(file); // read the position which is the size
    fseek(file, pos, SEEK_SET);  // restore original position
	
    int fileLength = (int) length; 	 // Get the size of the line

    line = malloc((fileLength + 1) * sizeof(char));
	if (fgets(line, fileLength, file) == NULL) {	// Error getting file
		fprintf(stderr, "Error getting line from file: %s\n", fileName);
		exit(1);
	}
	fclose(file);	// Close file

	return line;	
}

void checkValid(const char* pText) {
	int i;
	for (i = 0; i < strlen(pText); i++) {	//confirm all chars are valid
		if(!(pText[i] == ' ' || (pText[i] >= 'A' && pText[i] <= 'Z'))) {
			fprintf(stderr, "Input contains bad characters\n");
			exit(1);
		}
	}
}

void keyLengthCheck(int ptLen, int kLen, const char* keyFile) {
	if (ptLen > kLen) {	//checks to see if key is larger than plaintext
		fprintf(stderr, "Error: key '%s' is not long enough\n", keyFile);
		exit(1);
	}
}

void confirmType(int socketFD, int portNumber, char cType) {
	char response;

	send(socketFD, &cType, sizeof(char), 0);	//send client type for response from server

	recv(socketFD, &response, sizeof(char), 0);	// Receive response from server
	if (response != 't') {	//if server did not respond with t, than the wrong server was contacted
		close(socketFD);
		fprintf(stderr,"Port: %d | Incorrect Client/Server Interaction\n", portNumber); 
        exit(2);
	}	
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber, charsWritten, charsRead;
    char* key;
	char* eText;
	char* pText;
    char cType; //type of client (e or d) - encrypt or decrypt
    struct sockaddr_in serverAddress;

    cType = 'e'; //type of client is encryption (confirms correct server connections)
    // Check usage & args
    if (argc < 4) { 
        fprintf(stderr,"USAGE: %s filename keyfile port\n", argv[0]); 
        exit(0); 
    } 

    //text to be encrypted
	pText = fileContents(argv[1]);

	//key used for encryption
	key = fileContents(argv[2]);

	//gets port number and turns to int
	portNumber = atoi(argv[3]);

    //checks file for any invalid characters
    checkValid(pText);

    //checks key to see if is is long enough for the text
    keyLengthCheck(strlen(pText), strlen(key), argv[2]);

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0){
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
        error("CLIENT: ERROR connecting");
    }

    // Write to the server and recieve encoded msg
    confirmType(socketFD, portNumber, cType);
    eText = srMessage(pText, key, socketFD);
    printf("%s\n", eText);

    // Close the socket and deallocate mem
    close(socketFD);
    free(pText);
	free(eText);
	free(key);

    return 0;
}