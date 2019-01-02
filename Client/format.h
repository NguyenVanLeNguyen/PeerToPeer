#ifndef __FORMAT__
#define __FORMAT__
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
struct FileLocationRequest{
	char type;
	char FileName[8];
};

struct ListFileReport{
	char type;
	uint8_t quantity;
	char List[20][8];
	int size[20];
};

struct FileLocationRespond{
	unsigned int fileSize;
	uint8_t total;
	struct in_addr IP[256];
};

struct RequestData{
	char FileName[256];
	uint64_t StartByte;
	uint64_t FinishByte;
};

struct RespondData{
	uint32_t size;
	char md5[16];
};

struct AKC{
	char status;
};

#endif
