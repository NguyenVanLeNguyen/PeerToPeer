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
#include <stdlib.h>
#include <unistd.h>
//#include "kien.h"
#define DEFAULT_PORT 1508
#define serverport 8051
#define BLOCKSIZE 20000000

int blockNum;
int totalBlock;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char filenum[256];

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
	char num;

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

int downloadfile(int sockfd , char *filename, uint32_t start, uint32_t finish, FILE *fp) {
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
	printf("File name is :%s\n", filename);
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	w=write(sockfd,filename,strlen(filename));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	w=write(sockfd,&start,sizeof(uint32_t));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	w=write(sockfd,&finish,sizeof(uint32_t));
	if (w<0) {
		printf("%s\n","Write socket Error!" );
	}
	size_t fileSize=finish-start+1;
	//fseek (fp , start , SEEK_SET);
	while (fileSize>0) {
		r=read(sockfd,buffer,1024);
		fileSize-=r;
		buffer[r]='\0';
		fwrite(buffer,1,r,fp);
	}

}

void copyABlock(FILE *src,FILE * dst,int size) {
	char buffer[1024];
	int size2=size;
	while (size>0) {
		if (size>=1024) {
			fread(buffer, 1, 1024, src);
			fwrite(buffer,1,1024,dst);
		}
		else {
			fread(buffer, 1, size, src);
			fwrite(buffer,1,size,dst);
		}
		size-=1024;
	}
}

void *doit(void *arg){
	pthread_detach(pthread_self());
	struct threadVar tv= *((struct threadVar *) arg);
	int sockfd;
	struct sockaddr_in addr;
	FILE *fp;
	char c[3];
	char fileName[20];
	char tmpfile[20];
	strcpy(fileName,tv.fileName);
	strcpy(tmpfile,fileName);
	sprintf(c,"%d",(int)tv.num);
	strcpy(tmpfile+strlen(tmpfile),c);
	printf("tmpfile %s\n",tmpfile);
	fp=fopen(tmpfile,"a");
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
	addr.sin_addr = tv.IP;
	if( connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0 )
	{
		perror("connect");
		return 0;
	}
	else {
		printf("%s\n%d","connected!!!",(int)tv.num );
	}
	int num;
	while (1){
		pthread_mutex_lock(&mutex);
		num=blockNum;
		blockNum++;
		pthread_mutex_unlock(&mutex);
		if (num>=totalBlock) break;
		filenum[num]=tv.num;
		if (num<totalBlock-1) 
			downloadfile(sockfd,fileName,num*BLOCKSIZE,(num+1)*BLOCKSIZE-1,fp);
		else downloadfile(sockfd,fileName,num*BLOCKSIZE,tv.fileSize-1,fp);
		printf("Dowloading block %d , from %d to %d : ",num,num*BLOCKSIZE,(num+1)*BLOCKSIZE-1);
	}
	//downloadfile(sockfd,fileName,0,202412,fp);
	close(sockfd);
	fclose(fp);

}

int main(int argc, char *argv[]) {
	int clisockfd;
	int sockfd[20];
	int socketfd;
	struct sockaddr_in	servaddr;
	struct sockaddr_in addr;
	char srvIP[20];
	char tmpfile[20];
	char c[3];
	int srvPort;
	FILE *fp;
	FILE *fp2[10];
	uint64_t start;
	uint64_t finish;
	char str[20];
	char fileName[20];
	char rm[23];
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
	struct threadVar tv[10];
	uint32_t size=respond.fileSize;
	//printf("%d\n",size );
	if (size%BLOCKSIZE!=0)
		totalBlock=size/BLOCKSIZE+1;
	else totalBlock=size/BLOCKSIZE;
	printf("total block %d\n",totalBlock );
	pthread_t thread[20];
	printf("IP %s\n",inet_ntoa(respond.address[0]) );
	//printf("IP %s\n",inet_ntoa(respond.address[1]) );
	/*if((socketfd = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("socket!!!");
		return 0;
	}
	/*setup parameter for socket*/
	/*bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(1508);
	servaddr.sin_addr.s_addr =  inet_addr("127.0.0.1");
	if( connect(socketfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0 )
	{
		perror("connect");
		return 0;
	}
	fp2=fopen("anh1.png","w");
	downloadfile(socketfd,fileName,0,202412,fp2);
	fclose(fp2);*/

	for (int i=0;i<respond.numClient;i++) {
		tv[i].IP=respond.address[i];
		printf(" IP2 %s|\n",inet_ntoa(tv[i].IP) );
		tv[i].fileSize=size;
		strcpy(tv[i].fileName,fileName);
		tv[i].num=i;
		//downloadfile(respond->address[i].IP,1508,fileName,start,finish,fileName);
		pthread_create(&thread[i],NULL,&doit,(void *) &tv[i]);

	}
	for (int i=0;i<respond.numClient;i++) {
		pthread_join(thread[i],NULL);
	}

	//union file
	char fileNameTmp[20];
	for (int i=0;i<23; i++) rm[i]='\0';
	strcpy(rm,"rm ");
	strcpy(rm+3,fileName);
	system(rm);
	fp=fopen(fileName,"a");
	for (int i=0;i<respond.numClient;i++) {
		for(int i=0;i<20;i++) fileNameTmp[i]='\0';
		strcpy(fileNameTmp,fileName);
		sprintf(c,"%d",i);
		strcpy(fileNameTmp+strlen(fileNameTmp),c);
		fp2[i]=fopen(fileNameTmp,"r");
	}

	for (int i=0;i<totalBlock-1;i++) {
		copyABlock(fp2[filenum[i]],fp,BLOCKSIZE);
	}
	copyABlock(fp2[filenum[totalBlock-1]],fp,size%BLOCKSIZE);

	//remove tmp file
	for (int i=0;i<respond.numClient;i++) {
		for(int i=0;i<20;i++) fileNameTmp[i]='\0';
		strcpy(fileNameTmp,fileName);
		sprintf(c,"%d",i);
		strcpy(fileNameTmp+strlen(fileNameTmp),c);
		for (int i=0;i<23; i++) rm[i]='\0';
		strcpy(rm,"rm ");
		strcpy(rm+3,fileNameTmp);
		system(rm);
	}
	fclose(fp);
	for (int i=0;i<respond.numClient;i++) fclose(fp2[i]);

	printf("%s\n","Da tai xong, xin vui long kiem tra lai!" );
	close(clisockfd);

}