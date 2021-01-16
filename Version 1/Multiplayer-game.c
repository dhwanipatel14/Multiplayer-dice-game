#include<stdio.h>
#include <unistd.h>
#include<signal.h>
#include<time.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <stdlib.h>

//This is a Multiplayer dice game 
//Here there is a refree that communicates with all players
//It communicates with 1st player via pipes, 2nd with signals and third with fifos

void player1(char *ch, int *fd1, int *fd2);
void player2(int pid);
void player3(char *ch);
void signal_handler(int signo);


int main(int argc, char *argv[]){ 

    int fd1[2], fd2[2], fd3,fd4;  
	char turn='T';
	printf("This is a 3-player dice-game with a referee\n");  

    //First player uses pipes to communicate
	pipe(fd1); 
	pipe(fd2); 

	if(!fork()) { //Child process allows 1st player to play
		player1("1st player", fd1, fd2);
    }

	close(fd1[0]); //parent only writes to fd1 so reading end is close
	close(fd2[1]); //parent only reads from fd2 so writing end is close

//Second player communicates via signals
	int pid = fork();  
 	if(!pid) //child process for player2
	player2(pid);
	signal(SIGUSR1, signal_handler);

//Third player communicates via fifos
	unlink("../3rdplayer1.fifo");
	unlink("../3rdplayer2.fifo"); 
	
	if(mkfifo("../3rdplayer1.fifo", 0777)!=0){
        perror("Error in fifos.");
		exit(1);
    }
	if(mkfifo("../3rdplayer2.fifo", 0777)!=0){
        perror("Error in fifos.");
		exit(1);
    }

	if(!fork()) // child process for the 3rd  player
		player3("3rd player");

	fd3 = open("../3rdplayer1.fifo", O_WRONLY);  
	fd4 = open("../3rdplayer2.fifo", O_RDONLY);  

    //Coordination of three players
	while(1){ 
		printf("\nReferee: 1st player is playing\n\n");    
		write(fd1[1], &turn, 1); //parent writes to fd1
		read(fd2[0],  &turn, 1); //parent reads from fd2

		printf("\nReferee: 2nd player is playing\n\n");
		kill(pid, SIGUSR1); //sending sigusr1 signal to the 2nd player
		pause(); //waiting for 2nd player

		printf("\nReferee: 3rd player plays\n\n");  
		write(fd3, &turn, 1);//parent writes to fd3
		read(fd4,  &turn, 1);//parent reads from fd4

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
		read(fd1[0], &turn, 1);  //child read from fd1 or pipe1

		printf("%s: Rolling my dice\n", s);  
		dice =(int) time(&ss)%10 + 1;  
		printf("%s: Got %d points\n", s, dice); 
	     points+=dice;
		printf("%s: Total so far %d\n\n", s, points);  
		if(points >= 50){
			printf("%s: Game over I won, points=%d\n", s, points); 
              		kill(0, SIGTERM);
		}
		sleep(1);	//to slow down the execution  
		write(fd2[1], &turn, 1); //child write to fd2 or pipe2
	}
}

void signal_handler(int signo){
	static int points=0;	
	int dice;
	long int ss=0;  
	
	printf("2nd player: Rolling my dice\n");  
		dice =(int) time(&ss)%10 + 1;  
		printf("2nd player: Got %d points\n", dice); 
    	 points+=dice;
		printf("2nd player: Total so far %d\n\n", points);  
 	
		if(points >= 50){
			printf("3rd player: Game over I won, points=%d\n", points); 
              	kill(0, SIGTERM);
		}
		sleep(1);	//to slow down the execution  
		kill(getppid(),SIGUSR1); //wait up parent vis SIGUSR1
}

void player2(int pid) {  

	signal(SIGUSR1, signal_handler);
	while(1)
	 pause();
	  
}

void player3(char *s) {

	int fd1, fd2;

	fd1 = open("../3rdplayer1.fifo", O_RDONLY); 
	fd2 = open("../3rdplayer2.fifo", O_WRONLY);  

	static int points=0;
	int dice;
	long int ss=0;  
	
	char turn;  
	
	while(1){ 
		read(fd1, &turn, 1); //child read from fifo1

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

		write(fd2, &turn, 1); //child write to fifo2
	}

}
