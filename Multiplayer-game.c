#include<stdio.h>
#include <unistd.h>
#include<signal.h>
#include<time.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <stdlib.h>

//This is a Multiplayer dice game 

void player1(char *ch, int *fd1, int *fd2);
void player2(int pid);
void signal_handler(int signo);
void player3(char *ch);

int main(int argc, char *argv[]){ 

    int fd1[2], fd2[2];  
	char turn='T';

	printf("This is a 3-player game with a referee\n");  
	
	pipe(fd1); 
	pipe(fd2); 

	if(!fork()) {
		player("1st player", fd1, fd2);
    }

	close(fd1[0]); // parent not read from fd1,( parent only write to pipe 1 )
	close(fd2[1]);   // parent not write to fd2, ( parent only reads from pipe 2). 

	int pid=fork();  
 	if(!pid) //create the 2nd child process (2nd player)
	player2(pid);
	

	signal(SIGUSR1, signal_handler);

   //the code below is to create the 3rd player and its related fifo
	unlink("../3rdplayer1.fifo"); // delete it if it exists  
	unlink("../3rdplayer2.fifo"); // delete it if it exists  
	
	if(mkfifo("../3rdplayer1.fifo", 0777)!=0)
		exit(1);
	if(mkfifo("../3rdplayer2.fifo", 0777)!=0)
		exit(1);

	if(!fork()) //another child process, for the 3rd  player
		player3("3rd player");

	int fdp3 = open("../3rdplayer1.fifo", O_WRONLY);  
	int fdp4 = open("../3rdplayer2.fifo", O_RDONLY);  

  //the following loop is the referee coorinates the 3 players

	while(1){ 
		printf("\nReferee: 1st player plays\n\n");  
               
		write(fd1[1], &turn, 1);  //parent write to pipe 1, fd1
		read(fd2[0],  &turn, 1);

		printf("\nReferee: 2nd player plays\n\n");
		
		kill(pid, SIGUSR1);  //send sigusr1 signal to the 2nd player
		pause(); //parent pause for the 2nd player done


		printf("\nReferee: 3rd player plays\n\n");  
		write(fdp3, &turn, 1);
		read(fdp4,  &turn, 1);   

   }
}

void player1(char *s, int *fd1, int *fd2){  

	int points=0;
	int dice;
	long int ss=0;  
	
	char turn;  
	close(fd1[1]);
	close(fd2[0]);  
	
	while(1){ 
		read(fd1[0], &turn, 1);   //child read from pipe1 ,ie fd1

		printf("%s: rolling my dice\n", s);  
		dice =(int) time(&ss)%10 + 1;  
		printf("%s: got %d points\n", s, dice); 
	     points+=dice;
		printf("%s: Total so far %d\n\n", s, points);  
		if(points >= 50){
			printf("%s: game over I won, points=%d\n", s, points); 
              		kill(0, SIGTERM);
		}
		sleep(1);	// to slow down the execution  
		write(fd2[1], &turn, 1); //child write to pipe 2, ie fd2
	}
}

void signal_handler(int signo){
	static int points=0;	
	int dice;
	long int ss=0;  
	
	printf("2nd player: rolling my dice\n");  
		dice =(int) time(&ss)%10 + 1;  
		printf("2nd player: got %d points\n", dice); 
    	        points+=dice;
		printf("2nd player: Total so far %d\n\n", points);  
 	
		if(points >= 50){
			printf("3rd player: game over I won, points=%d\n", points); 
              	kill(0, SIGTERM);
		}
		sleep(1);	// to slow down the execution  
	
		kill(getppid(),SIGUSR1); //wait up parent vis SIGUSR1


}

