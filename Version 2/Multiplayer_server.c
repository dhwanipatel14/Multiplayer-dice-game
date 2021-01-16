#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
void servicePlayers(int,int);
int main(int argc, char *argv[]){
	char message[100];
	pid_t pid1;
	int sd, client1,client2, portNumber, i;
	int count=1;
	socklen_t len;
	struct sockaddr_in servAdd;
    
	if((sd = socket(AF_INET, SOCK_STREAM, 0))<0){
		fprintf(stderr, "Could not create socket\n");
		exit(1);
	}
	servAdd.sin_family = AF_INET;
	servAdd.sin_addr.s_addr = INADDR_ANY;
	sscanf(argv[1], "%d", &portNumber);
	servAdd.sin_port = 50000;
	bind(sd, (struct sockaddr *) &servAdd, sizeof(servAdd));
	listen(sd, 5);
	while(1){
		client1=accept(sd,(struct sockaddr*)NULL,NULL);
		printf("Player%d is ready\n", count);
		count ++;
		client2=accept(sd,(struct sockaddr*)NULL,NULL);
		printf("Player%d is ready\n", count);
		count ++;
		if(fork() == 0){
			servicePlayers(client1,client2);
		}
	}
}
void servicePlayers(int client1,int client2) {
 	int received_integer[2];
	int total[2]={0}, i =0;
	int client[2]={client1,client2};
	while(1) { 
		write(client[i%2], "You can play now", 100);
		if (read(client[i%2], &received_integer[i%2], sizeof(received_integer))<0){
			fprintf(stderr, "read() error\n");
			exit(3);
		}
		total[i%2] += ntohl(received_integer[i%2]); 
		if(total[i%2]>100){
			write(client[i%2], "Game over: you won the game", 100);
			write(client[(i+1)%2], "Game over: you lost the game", 100);
			break;
		}
		i++;
		sleep(1);
	}
	close(client[0]);
	close(client[1]);
	kill(getpid(), SIGTERM);
}
