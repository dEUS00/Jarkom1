#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define XON (0x11)
#define XOFF (0x13)

typedef enum { false=0, true } Boolean;
typedef unsigned char Byte;

Byte sent_xonxoff = XON;
Boolean send_xon = false, send_xoff = false;

typedef struct BUFFER
{
	int count;
	int head;
	int tail;
	char data[30];
} BUFFER;


void rcvchar(BUFFER *buf, char x) {
	if(buf->count==0) {
		buf->head = 0;
		buf->data[buf->head] = x;
		buf->count++;
	}
	else {
		buf->tail = buf->count;
		buf->data[buf->count] = x;
		buf->count++;
	}
}

void q_get(BUFFER *buf) {
	if (buf->count != 0) {
		printf("%c", buf->data[(buf->head) % 12]);
		buf->head++;
		buf->count--;	
	}
}

void init_buf(BUFFER *buf) {
	buf->count = 0;
	buf->head = 0;
	buf->tail = 0;
}

BUFFER buf;

int main()
{
	//Inisialisasi
	const char* hostname=0;
	const char* portname="2121";
	struct addrinfo hints;
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
	struct addrinfo* res=0;
	int err=getaddrinfo(hostname,portname,&hints,&res);
	if (err!=0) {
		return -1;
	}
	
	//Buat socket
	int fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if (fd==-1) {
		return -1;
	}
	
	//Bind ip ke socket
	if (bind(fd,res->ai_addr,res->ai_addrlen)==-1) {
		return -1;
	}
	
	freeaddrinfo(res);
	
	//Receive message
	char bridge[128];
	int recvlen;
	struct sockaddr_storage src_addr;
	init_buf(&buf);
	socklen_t src_addr_len=sizeof(src_addr);
	int i = 0;
	for(;;) {
		i++;
		recvlen = recvfrom(fd,bridge,sizeof(bridge),0,(struct sockaddr*)&src_addr,&src_addr_len);
		if (recvlen > 0) {
			bridge[recvlen] = 0;
			printf("Menerima byte ke-%d: '%c'\n", i, bridge[0]);
			if (bridge[0] != 'a')
				rcvchar(&buf, bridge[0]);
			else {
				char send[2];
				send[0] = XOFF;
				if (sendto(fd,send,sizeof(send),0,(struct sockaddr*)&src_addr,src_addr_len) != -1)
					printf("Mengirim XOFF\n");
				sleep(3);
				send[0] = XON;
				if (sendto(fd,send,sizeof(send),0,(struct sockaddr*)&src_addr,src_addr_len) != -1)
					printf("Mengirim XON\n");
			}	
		}
	}
	
	while(buf.count != 0)
		q_get(&buf);
	printf("\n");

	return 0;
}
