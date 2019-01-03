#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include "format.h"


static void *Handling(void *arg){
	int newfd;
	newfd = *( (int*) arg);
	free(arg);
	pthread_detach(pthread_self());
	unsigned char bufferr_receiver[2000];
	bzero(bufferr_receiver,sizeof(bufferr_receiver));
		/* code */

		bzero(bufferr_receiver, sizeof(bufferr_receiver));

		int rep = read(newfd,bufferr_receiver,sizeof(bufferr_receiver));
		printf("rep: %d bytes\n",rep );
		if((char)bufferr_receiver[0] == 1){ // FileLocationRequest (bufferr_receiver[0] == 32)
					printf("%s\n","FileLocationRequest: ");
					unsigned char packet_respond[2000];
					bzero(packet_respond, sizeof(packet_respond));
					int point_respon = 5;// location to save ip addresses in packet_respond
					char name[100],link[100] = "./";
					//struct in_addr IP[256];
					char num_of_host = 0;
					int size_of_file = 0;
					memcpy(&name,(bufferr_receiver+1),sizeof(bufferr_receiver)-1);

					strcpy(link, "./Save/");
					strcat(link,name);
					printf("%s\n",link);
					if(access(link,F_OK) == -1){


						printf("file not exists!");
						fflush(stdout);
						int zero = 0;
						char zero_char = 0;
						memcpy(packet_respond,(void *)&zero,sizeof(int));
						memcpy((packet_respond+4),(void *)&zero_char,sizeof(char));

					}
					else{
						FILE *name_file = fopen(link,"r");
						while(!feof(name_file)){


							struct in_addr ip;
							char line[30];
						//	int size_ = 0;
							fgets(line, 40,name_file);
							if (feof(name_file)){
									break;
							}
							//fscanf(name_file,"%s",line);
							printf("%s\n",line);
						//	fscanf(name_file,"%d",&size_);
							//fseek(name_file,1,SEEK_CUR);
							char *ipstr,*sizestr;
							 ipstr = (char *) malloc(sizeof(char) * 18);// maximum of ip addr is 15 characters
							 sizestr = (char *) malloc(sizeof(char) * 20);

							int space_index = strcspn (line," ");
							memcpy(ipstr,(void *)line,space_index);
							memcpy(sizestr,(line+space_index+1),(int)strlen(line)-space_index);

							int size_ = atoi(sizestr);
							if(size_ != size_of_file){
								size_of_file = size_;
								memcpy(packet_respond,&size_of_file,sizeof(int));// write size of file


							}
							inet_pton(AF_INET,ipstr,&ip);
							num_of_host++;
							memcpy((packet_respond+point_respon),&ip,sizeof(struct in_addr)); // write one ip address
							point_respon += sizeof(struct in_addr);
							printf("%s\n",ipstr);
							printf("%d\n",size_);
							free(ipstr);
							free(sizestr);



						}

						printf("num_of_host: %d\n",num_of_host );
						memcpy((packet_respond+4),&num_of_host,sizeof(char));// write total number of ip addresses
						fclose(name_file);

						printf("%d\n",point_respon);
						//printf("%s\n", );

					}
					write(newfd,packet_respond,sizeof(packet_respond));
		}

		else{ //ListFileReport
			printf("%s\n","ListFileReport: ");

			int point = 1;
			int quantity=0;
			memcpy(&quantity,bufferr_receiver+point,sizeof(int));
			point+=4;
			printf("%d\n",quantity);
			char size_name = 0;
			char name[255];
			int size_file = 0;

			struct sockaddr_in addr;
			socklen_t addr_size = sizeof(struct sockaddr_in);
			getpeername(newfd, (struct sockaddr *)&addr, &addr_size);
			char *clientip;
			clientip = (char *) malloc(sizeof(char)*20);
			strcpy(clientip, inet_ntoa(addr.sin_addr));
			for(int i = 0; i < rep;i++)
			printf("%c \n",bufferr_receiver[i]);
			for(int i = 0;i < quantity;i++){

					memcpy(&size_name,bufferr_receiver+point,sizeof(char));
					point+=sizeof(char);
					printf("%d\n",size_name);

					memcpy(name,bufferr_receiver+point,(int)size_name);
					point+=(int)size_name;
					printf("%s\n",name);

					memcpy(&size_file,bufferr_receiver+point,sizeof(int));
					point+=sizeof(int);
					printf("%d\n",size_file);

					char *link;
					link = (char *) malloc(sizeof(char)*255);
					strcpy(link, "./Save/");
					strcat(link,name);


					printf("%s\n",link);
					int flag = 1;
					if(!(access(link,F_OK) == -1)){
					FILE *nchek_file = fopen(link,"r");

					while(1){
						if(feof(nchek_file)){
							break;
						}
						char *line;
						line = (char *) malloc(sizeof(char) * 30);
						fgets(line, 30, (FILE*)nchek_file);
						char *ipstr;
						ipstr = (char *) malloc(sizeof(char) * 18);// maximum of ip addr is 15 characters

						int space_index = strcspn (line," ");
						memcpy(ipstr,(void *)line,space_index);
						if(strcmp(clientip,ipstr)){
							flag =0;
							free(ipstr);
							free(line);
							break;
						}
						free(ipstr);
						free(line);
					}
					fclose(nchek_file);
					}

					if(flag){
						FILE *name_file = fopen(link,"a");
						fprintf(name_file,"%s ",clientip);
						fprintf(name_file,"%d\n",size_file);
						fclose(name_file);
						free(link);
					}
					bzero(name,sizeof(name));
		}
	}
}
int main(int argc,char **agrv){
	int sockfd;
	int *newfd;

	pthread_t tid;
	struct sockaddr_in sockaddr,cliaddr;
	socklen_t cliaddr_len ;
	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("socket!!!");
		//return 0;
	}
	int enable = 1;
	//set sockopt to many socket use a same port
  if (setsockopt(sockfd, SOL_SOCKET,SO_REUSEPORT, &enable, sizeof(int)) < 0){
    	perror("setsockopt(SO_REUSEADDR) failed");
    }

	/*setup parameter for socket*/
	bzero(&sockaddr, sizeof(struct sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(14459);
	sockaddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
		perror("bind!!!");
	}

	if(listen(sockfd, 5) < 0) {
		perror("listen");
	}


	while(1){
			newfd = malloc(sizeof(int));
			cliaddr_len = sizeof(cliaddr);

			//printf("Waitting.... \n");
			//fflush(stdout);
			*newfd = accept(sockfd, (struct sockaddr*) &cliaddr, &cliaddr_len);
			if(*newfd < 0) { //error
				perror("accept");
				continue;
			}
			else{
		 		struct sockaddr_in* pVAddr = &cliaddr;
				struct in_addr ipAddr = pVAddr->sin_addr;
				char str[4];
				inet_ntop( AF_INET, &ipAddr, str, 32);
				unsigned short port;
				port = ntohs(pVAddr->sin_port);
				printf("%s %hu \n",str,port);
				pthread_create(&tid, NULL, &Handling, (void *) newfd);
			}



	}
	return 0;

}
