#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
//#include "kien.h"
#define clientport 1508
#define serverport 8051
#define BLOCKSIZE 8192

int blockNum;
int totalBlock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct FileLocationRequest
{
	char type;
	char filename[20];	
};

typedef struct FileLocationRespond
{
	uint64_t fileSize;
	char numClient;
	struct in_addr address[20];
};

typedef struct RequestData
{
	char fileName[20];
	uint64_t StartByte;
	uint64_t FinishByte;
};
typedef struct threadVar 
{
	int sockfd;
	int fileSize; 
	char fileName[20];
};

struct FileLocationRespond *requestListClient(char *srvIP, int srvPort, char *filename ) {
	int sockfd;
	struct sockaddr_in srv_addr;
	long IP;
	uint16_t bandwidth;
	//int r;
	//int w;
	struct FileLocationRespond *respond=malloc(sizeof(struct FileLocationRespond));
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (sockfd<0) {
		perror("Error:");
		//return NULL;
	}
	bzero((char *) &srv_addr, sizeof(srv_addr));
	srv_addr.sin_family=AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(srvIP);
	srv_addr.sin_port=htons(srvPort);
	if (connect(sockfd,(struct sockaddr *) &srv_addr,sizeof(srv_addr)) < 0) {	//Connect to server
		perror("Error connecting:");
		//return NULL;
	}
	char type=1	;
	write(sockfd,&type,sizeof(type));
	write(sockfd,filename,sizeof(filename));
	uint64_t fileSize;
	read(sockfd,&fileSize,sizeof(fileSize));
	respond->fileSize=fileSize;
	char numClient;
	read(sockfd,&numClient,sizeof(numClient));
	respond->numClient=numClient;
	for (int i=0;i<numClient;i++) {
		read(sockfd,&IP,sizeof(IP));
		read(sockfd,&bandwidth,sizeof(bandwidth));
		respond->address[i].IP=IP;
	}
	close(sockfd);
	return respond;
	

}

void *doit(void *arg){
	pthread_detach(pthread_self());
	struct threadVar tv= *((threadVar *) arg);

	while (blockNum<totalBlock){
		pthread_mutex_lock(&mutex);
		blockNum++;
		pthread_mutex_unlock(&mutex);
		if (blockNum>totalBlock) break;
		if (blockNum<totalBlock-1) 
			downloadfile(sockfd,tv.sockfd,tv.fileName,(blockNum-1)*BLOCKSIZE,blockNum*BLOCKSIZE-1,tv.filename);
		else downloadfile(sockfd,tv.sockfd,tv.fileName,blockNum*BLOCKSIZE,tv.fileSize-1,tv.filename);
		
	}

}

int downloadfile(int sockfd, char *filename, uint64_t start, uint64_t finish, char *tmpfile) {
	/*int sockfd;
	struct sockaddr_in addr;
	char buffer[1024];
	int r;
	int w;
	int l;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (sockfd<0) {
		perror("Error:");
		return 1;
	}
	bzero((char *) &addr, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr = IP;
	addr.sin_port=htons(port);
	if (connect(sockfd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {	//Connect to server
		perror("Error connecting:");
		return 1;
	}*/
	l=strlen(filename);
	w=write(sockfd,&l,sizeof(int));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	w=write(sockfd,filename,strlen(filename));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	w=write(sockfd,&start,sizeof(uint64_t));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	w=write(sockfd,&finish,sizeof(uint64_t));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	size_t fileSize=finish-start+1;
	FILE *fp;
	fp=fopen(tmpfile,"a");
	fseek (fp , start , SEEK_SET);
	while (fileSize>0) {
		r=read(sockfd,buffer,1024);
		fileSize-=r;
		buffer[r]='\0';
		fwrite(buffer,1,r,fp);
	}
	fclose(fp);
	close(sockfd);

}

int main(int argc, char *argv[]) {
	char fileName[20];
	char srvIP[20];
	char str[20];
	struct threadVar tv;
	int srvPort;
	struct sockaddr_in addr;
	int sockfd[20];
	FILE *fp;
	uint64_t start;
	uint64_t finish;
	fp=fopen("kien.conf","r");
	printf("%s\n","Xin chao moi nguoi, cam on da su dung phan mem download file \"kien v1.0\" !, moi van de xin lien he tytotum0003@gmail.com");
	while (fscanf(fp,"%s",str)!= EOF) {
		if (strcmp(str,"indexServerIP")==0) {
			fscanf(fp,"%s",srvIP);
		}
		else if (strcmp(str,"indexServerPort")==0) {
			fscanf(fp,"%d",&srvPort);
		}
	}
	fclose(fp);
	while(1==1) {
		printf("%s\n","Xin moi ban nhap ten file: " );
		scanf("%s",fileName);
		printf("%s\n","Dang tai file, xin vi long cho!" );
		struct FileLocationRespond *respond;
		respond=requestListClient(srvIP,srvPort,fileName);
		uint64_t size=respond->fileSize;
		if (size%BLOCKSIZE!=0)
			totalBlock=size/BLOCKSIZE+1;
		else totalBlock=size/BLOCKSIZE;
		pthread_t thread[20];
		for (int i=0;i<respond->numClient;i++) {
			sockfd[i]=socket(AF_INET,SOCK_STREAM,0);
			if (sockfd[i]<0) {
				perror("Error:");
				return 1;
			}
			bzero((char *) &addr, sizeof(addr));
			addr.sin_family=AF_INET;
			addr.sin_addr.s_addr = respond->address[i].IP;
			addr.sin_port=htons(1508);
			if (connect(sockfd[i],(struct sockaddr *) &addr,sizeof(addr)) < 0) {	//Connect to server
				perror("Error connecting:");
				return 1;
			}
		}
		for (int i=0;i<respond->numClient;i++) {
			tv.sockfd=sockfd[i];
			tv.fileSize=size;
			strcpy(tv.fileName,fileName);
			//downloadfile(respond->address[i].IP,1508,fileName,start,finish,fileName);
			pthread_create(&thread[i],NULL,&doit,(void *)tv);
		}
		printf("%s\n","Da tai xong, xin vui long kiem tra lai!" );
	}

}