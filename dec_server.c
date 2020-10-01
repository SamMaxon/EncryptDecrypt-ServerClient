#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


void error(const char *msg) {
	perror(msg);
  	exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  	// Clear out the address struct
  	memset((char*) address, '\0', sizeof(*address)); 

  	// The address should be network capable
  	address->sin_family = AF_INET;
  	// Store the port number
  	address->sin_port = htons(portNumber);
	 
  	// Allow a client at any address to connect to this server
  	address->sin_addr.s_addr = INADDR_ANY;
}

char* decode(char* encodedText, char* key) {
	int r; // ASCII number
	char* plainText = malloc((strlen(encodedText)) * sizeof(char));
	int i;
	for (i = 0; i < strlen(encodedText); i++) {
		if (encodedText[i] == ' ') encodedText[i] = 91;	 // space is now 91 ascii for easily encoded messages

		if (key[i] == ' ') key[i] = 91;
		r = encodedText[i] - key[i];	
		if (r < 0)	
			r += 27;
		else
			r %= 27;
		r += 65;
		if (r == 91)
			plainText[i] = ' ';
		else
			plainText[i] = r;
	}
    plainText[i] = '\0';
	return plainText;	// return decoded text
}

void sendDecryptedMessage(int connectionSocket) {
	char* key;	
	char* plainText;
	char* encodedText;	
	int msgLength;

	// Get the text length from client
	recv(connectionSocket, &msgLength, sizeof(int), 0); 		

	// Receive encrypted text from client
	encodedText = malloc((msgLength + 2) * sizeof(char));
	recv(connectionSocket, encodedText, msgLength * sizeof(char), 0);

	// Receive key from client
	key = malloc((msgLength + 2) * sizeof(char));
    recv(connectionSocket, key, msgLength * sizeof(char), 0); 

	plainText = decode(encodedText, key);
	// Send back encrypted text
	send(connectionSocket, plainText, msgLength * sizeof(char), 0); 
		
	free(key);
	free(plainText);
	free(encodedText);
}

void confirmType(int connectionSocket, char Type) {
	char in, response;

	recv(connectionSocket, &in, sizeof(int), 0);

	if (in == Type) {	//check to see if correct type of connection
		response = 't';	//reply with true
		send(connectionSocket, &response, sizeof(char), 0);
	}
	else {	// Not the correct client
		response = 'f'; //reply with false
		send(connectionSocket, &response, sizeof(char), 0);
		close(connectionSocket); // Close the existing socket which is connected to the client
		exit(1);
	}
}

int main(int argc, char *argv[]){
  	int connectionSocket;
	char sType; 
  	struct sockaddr_in serverAddress, clientAddress;
  	socklen_t sizeOfClientInfo = sizeof(clientAddress);

	sType = 'd'; //type of server is decryption (confirms correct client connections)
	// Check usage & args
  	if (argc < 2) { 
    	fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    	exit(1);
  	} 
  	
  	// Create the socket that will listen for connections
  	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  	if (listenSocket < 0) {
    	error("ERROR opening socket");
  	}

	
  	// Set up the address struct for the server socket
  	setupAddressStruct(&serverAddress, atoi(argv[1]));

  	// Associate the socket to the port
  	if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
    	error("ERROR on binding");
  	}

  	// Start listening for connetions. Allow up to 5 connections to queue up
  	listen(listenSocket, 5); 
  
  	// Accept a connection, blocking if one is not available until one connects
  	while(1){
    	// Accept the connection request which creates a connection socket
		connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    	if (connectionSocket < 0){
      		error("ERROR on accept");
    	}
    	printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
    	
		pid_t pid = fork();
		
		if (pid < 0) {
			fprintf(stderr, "Problem creating child\n");
			exit(1);
		}else if (pid ==0) {	// Child 
			confirmType(connectionSocket, sType); 
			sendDecryptedMessage(connectionSocket);
			exit(0); //kill child
		}

    	close(connectionSocket); 
  	}
  	// Close the listening socket
  	close(listenSocket); 
  	return 0;
}
