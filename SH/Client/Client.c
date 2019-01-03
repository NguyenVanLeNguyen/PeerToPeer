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
#include <pthread.h>
#include <dirent.h>
//#include "kien.h"
#define DEFAULT_PORT 1508
#define serverport 8051
#define BLOCKSIZE 8192

int blockNum;
int totalBlock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct FileLocationRequest
{
	char type;
	char filename[20];	
};

struct FileLocationRespond
{
	uint32_t fileSize;
	char numClient;
	struct in_addr address[20];
};

struct RequestData
{
	char fileName[20];
	uint64_t StartByte;
	uint64_t FinishByte;
};

struct threadVar 
{
	struct in_addr IP;
	int fileSize; 
	char fileName[20];
	FILE *fp;
};

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

struct FileLocationRespond Request_dowload(int *sockfd, char *name){
	struct FileLocationRespond respond_from_client;
	char request[258];
	char respond[200];
	request[0] = 1;
	memcpy((request+1),name,strlen(name));
	printf("%s|\n",name );
	write(*sockfd,request,strlen(name)+1);

	int rep = read(*sockfd,respond,sizeof(respond));
	printf("rep: %d\n",rep);
	int size_of_file = 0;
	memcpy(&size_of_file,respond,sizeof(int));
	//printf("size: %d\n",size_of_file);
	respond_from_client.fileSize = size_of_file;
	char numh = 0;
	memcpy(&numh,respond+4,sizeof(char));

	respond_from_client.numClient = (int)numh;

	//struct in_addr list_host[num_of_host];
	for(int i = 0; i < respond_from_client.numClient;i++){
		memcpy((respond_from_client.address+i),respond+5+(i*4),sizeof(struct in_addr));

	}
	//respond_from_client.IP = list_host;
	return respond_from_client;
}

/*struct FileLocationRespond *requestListClient(char *srvIP, int srvPort, char *filename ) {
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
*/

void *doit(void *arg){
	pthread_detach(pthread_self());
	struct threadVar tv= *((struct threadVar *) arg);
	printf("abc%s\n",inet_ntoa(tv.IP) );
	int sockfd;
	struct sockaddr_in addr;
	FILE *fp;
	fp=fopen(tv.fileName,"rb+");
	printf("abc%s\n",inet_ntoa(tv.IP) );
	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("socket!!!");
		return 0;
	}
	else {
		printf("%s\n","connected1!!!" );
	}

	/*setup parameter for socket*/
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1508);
	//addr.sin_addr.s_addr =  tv.IP.s_addr;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if( connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0 )
	{
		perror("connect");
		return 0;
	}
	else {
		printf("%s\n","connected!!!" );
	}
	char fileName[20];
	strcpy(fileName,tv.fileName);
	while (blockNum<totalBlock){
		pthread_mutex_lock(&mutex);
		blockNum++;
		pthread_mutex_unlock(&mutex);
		if (blockNum>totalBlock+1) break;
		if (blockNum<totalBlock-1) 
			downloadfile(sockfd,fileName,(blockNum-1)*BLOCKSIZE,blockNum*BLOCKSIZE-1,fp);
		else downloadfile(sockfd,fileName,(blockNum-1)*BLOCKSIZE,tv.fileSize-1,fp);
		
	}
	close(sockfd);
	fclose(fp);

}

int downloadfile(int sockfd , char *filename, uint64_t start, uint64_t finish, FILE *fp) {
	//int sockfd;
	char buffer[1024];
	int r;
	int w;
	int l;
	/*sockfd=socket(AF_INET,SOCK_STREAM,0);
	if (sockfd<0) {
		perror("Error:");
		return 1;
	}
	bzero((char *) &addr, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr = IP;
	addr.sin_port=htons(DEFAULT_PORT);
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
	fseek (fp , start , SEEK_SET);
	while (fileSize>0) {
		r=read(sockfd,buffer,1024);
		fileSize-=r;
		buffer[r]='\0';
		fwrite(buffer,1,r,fp);
	}
	fclose(fp);

}

int main(int argc, char *argv[]) {
	/*char fileName[20];
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
	}*/
	int clisockfd;
	int sockfd[20];
	struct sockaddr_in	servaddr;
	struct sockaddr_in addr;
	char srvIP[20];
	int srvPort;
	FILE *fp;
	FILE *fp2;
	uint64_t start;
	uint64_t finish;
	char str[20];
	char fileName[20];
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
	printf("%d\n",srvPort);
	printf("%s\n",srvIP );
	if((clisockfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("socket!!!");
		return 0;
	}
	/*setup parameter for socket*/
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(srvPort);
	servaddr.sin_addr.s_addr =  inet_addr(srvIP);
	if( connect(clisockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 )
	{
		perror("connect");
		return 0;
	}
	printf("%s","Moi ban nhap ten file: " );
	scanf("%s",fileName);

	struct FileLocationRespond respond=Request_dowload(&clisockfd,fileName);
	printf("aaahaaha%d\n",respond.numClient);
	struct threadVar tv;
	uint32_t size=respond.fileSize;
	//printf("%d\n",size );
	if (size%BLOCKSIZE!=0)
		totalBlock=size/BLOCKSIZE+1;
	else totalBlock=size/BLOCKSIZE;
	pthread_t thread[20];

	for (int i=0;i<respond.numClient;i++) {
		tv.IP=respond.address[i];
		tv.fileSize=size;
		strcpy(tv.fileName,fileName);
		//downloadfile(respond->address[i].IP,1508,fileName,start,finish,fileName);
		pthread_create(thread+i,NULL,&doit,(void *) &tv);
	}
	
	printf("%s\n","Da tai xong, xin vui long kiem tra lai!" );
	close(clisockfd);

}