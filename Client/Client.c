#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
#include "format.h"
long sizes(FILE *file){
	fseek(file,0,SEEK_END);
	long value = ftell(file);
	fseek(file,0,SEEK_SET);
	return value;
}

int Creat_ClientUpdateFileLocation_Packet(int *sockfd){
	struct dirent *de=NULL;
	DIR *d=NULL;
	unsigned char Report[2000];
	bzero(Report,2000);
	Report[0] = 2;
	d=opendir("Data");
	if(d == NULL)
	{
	   perror("Couldn't open directory");
		   return 0;
	}
	int numbe_of_file = 0;
	const int *ptrnumber = &numbe_of_file;
	char lname = 0;
  const char *ptrlname = &lname;
	int size =0;
	const int *ptrsize = &size;
	int point = 5;

	while(de = readdir(d)){

		if(de->d_type == DT_REG){
			numbe_of_file++;


			lname = (char) strlen(de->d_name);
			printf("%s %d\n",de->d_name,lname);

			FILE *ofile;
	    char *name,*namef;

	    name = (char *) malloc(100);
	    namef = (char *) malloc(100);
	    strcpy(name,de->d_name);
	    strcpy(namef, "./Data/");
	    strcat(namef,name);
	    ofile = fopen(namef,"rb");
	    size = (int)sizes(ofile);
        bcopy(&lname,(Report+point),sizeof(char));
        point++;
        bcopy(name,(Report+point),(int)lname+1);
				point = point + (int)lname;
        bcopy(&size,(Report+point),sizeof(int));
        point = point + sizeof(int);
        free(name);
		    free(namef);
		    fclose(ofile);

	  }

	}
		bcopy(&numbe_of_file,(Report+1),sizeof(int));
    printf("%ld\n",strlen(Report));
    printf("%d\n\n",Report[1] );
		write(*sockfd,Report,point);
     return point;
	}
struct FileLocationRespond Request_dowload(int *sockfd){
	struct FileLocationRespond respond_from_client;
	char request[258];
	char respond[200];
	char name[256];

	request[0] = 1;
	printf("Please enter the file's name: ");
	bzero(name,sizeof(name));
	bzero(respond,sizeof(respond));
	scanf("%s",name);
	printf("%s\n",name);
	memcpy((request+1),name,strlen(name));
	write(*sockfd,request,strlen(name)+1);

	int rep = read(*sockfd,respond,sizeof(respond));
	for(int i = 0;i < rep;i++){
		printf("%c ", respond[i]);
	}
	printf("\n" );

	printf("rep: %d\n",rep);
	int size_of_file = 0;
	memcpy(&size_of_file,respond,sizeof(int));
	//printf("size: %d\n",size_of_file);
	respond_from_client.fileSize = size_of_file;
	char numh = 0;
	memcpy(&numh,respond+4,sizeof(char));

	respond_from_client.total = (int)numh;



	//struct in_addr list_host[num_of_host];
	for(int i = 0; i < respond_from_client.total;i++){
		memcpy((respond_from_client.IP+i),respond+5+(i*4),sizeof(struct in_addr));

		char *clientip;
		clientip = (char *) malloc(sizeof(char)*20);
		strcpy(clientip, inet_ntoa(respond_from_client.IP[i]));
		printf("%s\n",clientip );
	}
	//respond_from_client.IP = list_host;
	return respond_from_client;
}

int main(int argc,char **agrv){

	char buffsz[10];

	int	sockfd;

	struct sockaddr_in	servaddr;
	char ipsever[20];
	printf("Please enter the ipsever: ");
	bzero(ipsever,sizeof(ipsever));
	fgets(ipsever,sizeof(ipsever),stdin);
	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("socket!!!");
		return 0;
	}

	/*setup parameter for socket*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(14459);
	servaddr.sin_addr.s_addr =  inet_addr(ipsever);


	/*check and set buffer's size*/
	// int show_buffer_size = 0;
	// socklen_t	optlen;
	// optlen = sizeof(show_buffer_size);
	// if(getsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, &show_buffer_size, &optlen) < 0)
	// perror("SO_RCVBUF getsockopt error");
	// printf("check buffer's size defaults : %d\n",show_buffer_size);
	// printf("buffer's size: ");
	// bzero(buffsz,sizeof(buffsz));
	// fgets(buffsz,sizeof(buffsz),stdin);
	// int buffsize = atol(buffsz);
	// if (setsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, &buffsize, sizeof(buffsize)) < 0){
	// 		perror("setsockopt SO_RCVBUF failed");
	// }
	// if(getsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, &show_buffer_size, &optlen) < 0)
	// 	perror("SO_RCVBUF getsockopt error");
	// printf("check buffer's size (after setting it): %d\n",show_buffer_size);


	printf("choose option: 1 <update list file> ; 2 <download file>; 3 <quit> \n option:");
	int option = 0;
	scanf("%d",&option);

	/**/
	while(1){
		if(option == 1){
			/*code for update list file feature*/
			if( connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 )
			{
				perror("connect");
				return 0;
			}
			//char Report[2000];
			int size_of_report = Creat_ClientUpdateFileLocation_Packet(&sockfd);

			close(sockfd);
			return 0;

		}
		else if(option == 2){
			/*code for send request to download file */
			if( connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 )
			{
				perror("connect");
				return 0;
			}

			struct FileLocationRespond respond_from_client = Request_dowload(&sockfd);
			printf("%d\n",respond_from_client.fileSize);

			// for(int i = 0; i < respond_from_client.total;i++){
			//
			//
			// }
			close(sockfd);
			return 0;


		}
		else if(option == 3){
			return 0;
		}
		else{
			/**/
			printf("wrong option");

		}
	}

	return 0;

}
