#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>	
#include <sys/stat.h>

char* kgenerate(int klength) {
	int r; //ascii number 
	int i;
    char* k = malloc((klength + 2) * sizeof(char)); //adds 2 for newline and terminator

	for (i = 0; i < klength; i++) {
		
        r = rand() % 27 + 65; // Random number from 65-91

		if (r == 91) 
            r = 32;     // Change '[' to space character
		
        k[i] = r;
	}
	// End k with a new line
	k[i] = '\n';
	
	return k;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {     //checks for the correct amount of arguments from command line
		fprintf(stderr, "Error: Too Few Arguments\n");
		exit(1);
	}
	
	srand(time(NULL)); //time seed

	int klength = atoi(argv[1]); //gets length from argument and turns it into int 
	char* k = kgenerate(klength);
	printf("%s", k);	
	
	free(k);

	return 0;
}