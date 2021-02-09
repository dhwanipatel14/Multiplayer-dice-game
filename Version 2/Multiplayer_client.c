#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
char message[100];
long int ss=0;
int dice = 0;
int converted_number, number_to_send;
int csd, portNumber;
socklen_t len;
struct sockaddr_in servAdd;

	if ((csd =socket(AF_INET,SOCK_STREAM,0))<0){
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	servAdd.sin_family = AF_INET;
	servAdd.sin_addr.s_addr = inet_addr(argv[1]);

	}
	if(connect(csd, (struct sockaddr *) &servAdd, sizeof(servAdd))<0){
		fprintf(stderr, "connect() failed, exiting\n");
		exit(3);
	}
	while(1) {
		if (read(csd, message, 100)<0){
			fprintf(stderr, "read() error\n");
			exit(3);
		}
		if (strcmp(message, "You can play now")==0){
			fprintf(stderr, "%s\n", message);
			dice =(int) time(&ss)%10 + 1;
			printf("Obtained Score: %d\n", dice);
			converted_number = htonl(dice);
			write(server, &converted_number, sizeof(converted_number));
		}
		if (strcmp(message, "Game over: you won  game")==0){
 			fprintf(stderr, "%s\n", "I won the game");
			break;
		}
		else if (strcmp(message,"Game over: you lost the game")==0){
			fprintf(stderr, "%s\n", "I lost the game");
			break;
		}
	}
	exit(0);
}
