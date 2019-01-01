#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
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
		   return NULL;
	}
	int numbe_of_file = 0;
	const int *ptrnumber = &numbe_of_file;
	char lname = 0;
  const char *ptrlname = &lname;
	int size =0;
	const int *ptrsize = &size;
	int point = 1;
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



		  /*bcopy(&lname,(void *)ClientUpdateFileLocation[point],sizeof(char));
			point++;
			bcopy(namef,(void *)ClientUpdateFileLocation[point],(int)lname);
			point+=(int)lname;
			bcopy(&size,(void *)ClientUpdateFileLocation[point],sizeof(int));
			point+=sizeof(int);
      memcpy((ClientUpdateFileLocation+point),(char*)&lname,sizeof(char));
			point++;
      printf("%d \n",point);
			memcpy((ClientUpdateFileLocation+point),(char*)name,(int)lname+1);
			point = point + (int)lname;
      printf("%d \n",point);

			memcpy((ClientUpdateFileLocation+point),(char*)&size,sizeof(int));
			point = point + sizeof(int);
      free(name);
	    free(namef);
	    fclose(ofile);
      printf("%d \n",point);*/
        bcopy(&lname,(ClientUpdateFileLocation+point),sizeof(char));
        point++;
        printf("%d \n",point);
        bcopy(name,(ClientUpdateFileLocation+point),(int)lname+1);
				point = point + (int)lname;
        printf("%d \n",point);

        bcopy(&size,(ClientUpdateFileLocation+point),sizeof(int));
        point = point + sizeof(int);
        free(name);
	    free(namef);
	    fclose(ofile);
      printf("%d \n",point);
	  }

	}
  //bcopy(&numbe_of_file,(void *)ClientUpdateFileLocation[1],sizeof(int));
 //memcpy(&ClientUpdateFileLocation[1],(char *)&numbe_of_file,sizeof(int));
// ClientUpdateFileLocation[point+1] = '\0';
    printf("%ld\n",strlen(ClientUpdateFileLocation));
    printf("%d\n\n",ClientUpdateFileLocation[1] );
    //printf("%d\n",ClientUpdateFileLocation[1] );
    for(int i =0; i <= point;i++)
      if(ClientUpdateFileLocation[i] == '\0')
        printf("%d\n",i);
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
			Creat_ClientUpdateFileLocation_Packet("Data");


		}
		else if(option == 2){
			/*code for send request to download file */
			if( connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 )
			{
				perror("connect");
				return 0;
			}
			char name[100] = "./";
			char mode[2] = "wb";
			char buffer[256];

			struct FileLocationRequest request_pkt ;
			request_pkt.type = '1';
			printf("Please enter the file's name: ");
			bzero(buffer,sizeof(buffer));
			strcpy(name, "./");
			fgets(buffer,256,stdin);



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
