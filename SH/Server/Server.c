#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

int totalFile=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *doit(void *arg){
	//char buffer_read[1024];	//
	char buffer[1024];		//
	int lSize,sizeToRead;	//
	int blockSize;
	int BLOCK_SIZE=1023;
	int sum=0;	//total bytes send
	FILE * fp; //File to send to Client
	int r,w;
	size_t result;
	uint32_t start,finish;
	pthread_detach(pthread_self());
	char status;
	char filename[256];
	int l;	//length of filename
	//str_echo(* ( ( int *) arg)); /* same function as before */
	//int clientsockfd;

	int clientsockfd= *((int *) arg);
	printf("%d\n",clientsockfd);
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	getpeername(clientsockfd, (struct sockaddr *)&addr, &addr_size);
	printf("address %s\n",inet_ntoa(addr.sin_addr) );
	sum=0;	
	while (1) {
		for (int i=0;i<256;i++) filename[i]='\0'; 	//init buffer=null
				//for (int i=0;i<1024;i++) buffer_write[i]='\0'; 
		read(clientsockfd,&l,sizeof(int));
		r = read(clientsockfd,filename,l);	//Read message from client by socket
			if (r <= 0) {
			perror("Error reading from socket!!");
			break;
		}
	
		printf("Client need file:%s\n",filename);
		read(clientsockfd,&start,sizeof(uint32_t));
		read(clientsockfd,&finish,sizeof(uint32_t));
		printf("%d %d\n",start,finish);
		fp=fopen(filename,"r");
		if (fp==NULL) {
        	status=1;
        	printf("%s\n","not found!" );
			//write(clientsockfd, &status, 1);
        //return ;
    	}
    //status=2;
	//write(clientsockfd, &status, 1);
		fseek (fp , start , SEEK_SET);
    //lSize = ftell (fp);
    //rewind (fp);
    	sizeToRead = finish-start+1;
    	printf("%d\n",sizeToRead);
	//write(clientsockfd, &lSize, 4);
	//printf("The size of the file is:%d\n",lSize);
		while(sizeToRead > 0){
	        if (sizeToRead >= BLOCK_SIZE){
	            blockSize = BLOCK_SIZE;
	        }
	        else{
	            blockSize = sizeToRead;
	        }
	        result = fread(buffer, 1, blockSize, fp);
	        //printf("Size %d\n",blockSize);
			buffer[result] = '\0';
			w=write(clientsockfd,buffer,result);
			sum=sum+w;
	        sizeToRead -= result;
	    }
		printf("Total bytes sent:%d\n",sum);	
		fclose(fp);
	}
	pthread_mutex_lock(&mutex);
	totalFile++;
	printf("Total File download:%d\n",totalFile);
	pthread_mutex_unlock(&mutex);
	close(clientsockfd);	
	close(* ( ( int *) arg)); /* done with connected socket */
}

int main(int argc, char *argv[]) {
	int sockfd;	// Server listen socket descripter
	int *clientsockfd;	//Client connect socket descripter
	struct sockaddr_in serv_addr, cli_addr;	//Server address and Client address 
	socklen_t length;
	pthread_t tid;	
		//
	//unsigned int SIGCHLD;
	//signal_t SIGCHLD;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	//Create a socket IPv4
    if (sockfd < 0) {
        perror("Error while creating socket!");
		return 1;
	}
	int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;		//IP v4
    serv_addr.sin_addr.s_addr = INADDR_ANY;	//default IP address
    serv_addr.sin_port = htons(1508);	//Port 8899

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { 	//Associate socket with IP address and port number
        perror("Error while binding");
		return 1;
	}
	listen(sockfd,10);	//Listen in sockfd, Accept for 5 connects
	while(1) {
		length = sizeof(cli_addr);
		clientsockfd= malloc(sizeof(int));
		//clientsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &length);		//Accept connect from client
		if ( (*clientsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &length))<0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				perror("Accept error!");

		}
		pthread_create(&tid, NULL, &doit, (void *) clientsockfd);
	}		
	close(sockfd);
	return 0;
}