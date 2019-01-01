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

int Creat_ClientUpdateFileLocation_Packet(char *ClientUpdateFileLocation){
	struct dirent *de=NULL;
	DIR *d=NULL;
	//unsigned char ClientUpdateFileLocation[2000];
	bzero(ClientUpdateFileLocation,2000);
	ClientUpdateFileLocation[0] = 2;
	d=opendir("Server");
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
//  memcpy((ClientUpdateFileLocation+1),&numbe_of_file,sizeof(int));

	while(de = readdir(d)){

		if(de->d_type == DT_REG){
			numbe_of_file++;


			lname = (char) strlen(de->d_name);
			printf("%s %d\n",de->d_name,lname);

			FILE *ofile;
	    char *name,*namef;
			//const char *ptrnamef = namef;
	    name = (char *) malloc(100);
	    namef = (char *) malloc(100);
	    strcpy(name,de->d_name);
	    strcpy(namef, "./Server/");
	    strcat(namef,name);
	    //printf("%s ",namef );
	    ofile = fopen(namef,"rb");
	    size = (int)sizes(ofile);
        bcopy(&lname,(ClientUpdateFileLocation+point),sizeof(char));
        point++;
        //printf("%d \n",point);
        bcopy(name,(ClientUpdateFileLocation+point),(int)lname+1);
				point = point + (int)lname;
      //  printf("%d \n",point);

        bcopy(&size,(ClientUpdateFileLocation+point),sizeof(int));
        point = point + sizeof(int);
        free(name);
		    free(namef);
		    fclose(ofile);
	     // printf("%d \n",point);
	  }

	}
		bcopy(&numbe_of_file,(ClientUpdateFileLocation+1),sizeof(int));
   //memcpy(&ClientUpdateFileLocation[1],(char *)&numbe_of_file,sizeof(int));

    printf("%ld\n",strlen(ClientUpdateFileLocation));
    printf("%d\n\n",ClientUpdateFileLocation[1] );
    //printf("%d\n",ClientUpdateFileLocation[1] );
    //for(int i =0; i <= point;i++)
    //if(ClientUpdateFileLocation[i] == '\0')
        //printf("%d\n",i);
     return point;
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
	int show_buffer_size = 0;
	socklen_t	optlen;
	optlen = sizeof(show_buffer_size);
	if(getsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, &show_buffer_size, &optlen) < 0)
	perror("SO_RCVBUF getsockopt error");
	printf("check buffer's size defaults : %d\n",show_buffer_size);
	printf("buffer's size: ");
	bzero(buffsz,sizeof(buffsz));
	fgets(buffsz,sizeof(buffsz),stdin);
	int buffsize = atol(buffsz);
	if (setsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, &buffsize, sizeof(buffsize)) < 0){
			perror("setsockopt SO_RCVBUF failed");
	}
	if(getsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, &show_buffer_size, &optlen) < 0)
		perror("SO_RCVBUF getsockopt error");
	printf("check buffer's size (after setting it): %d\n",show_buffer_size);


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
			char Report[2000];
			int size_of_report = Creat_ClientUpdateFileLocation_Packet(Report);
			write(sockfd,Report,size_of_report);
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
			char Request[258];
			char Respon[200];
			char buffer[256];

			Request[0] = 1;
			printf("Please enter the file's name: ");
			bzero(buffer,sizeof(buffer));
			scanf("%s",buffer);
			printf("%s\n",buffer);
			memcpy((Request+1),buffer,strlen(buffer));
			write(sockfd,Request,strlen(buffer)+1);

			int rep = read(sockfd,Respon,sizeof(Respon));
			printf("rep: %d\n",rep);
			int size_of_file = 0;
			memcpy(&size_of_file,Respon,sizeof(int));
			printf("size: %d\n",size_of_file);
			char numh = 0;
			memcpy(&numh,Respon+4,sizeof(char));
			int num_of_host = 0;
			num_of_host = (int)numh;
			printf("%d\n",num_of_host);

			struct in_addr list_host[num_of_host];
			for(int i = 0; i < num_of_host;i++){
				memcpy((list_host+i),Respon+5+(i*4),sizeof(struct in_addr));
				char *clientip;
				clientip = (char *) malloc(sizeof(char)*20);
				strcpy(clientip, inet_ntoa(list_host[i]));
				printf("%s\n",clientip );
			}

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
