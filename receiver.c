#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

typedef struct BUFFER
{
	int count;
	char data[30];
} BUFFER;

void rcvchar(BUFFER *buf, char x) {
	buf->data[buf->count] = x;
	buf->count++;
}

void q_get(BUFFER *buf) {
	printf("%c\n", buf->data[(buf->count-1) % 12]);
	buf->count--;
}

void init_buf(BUFFER *buf) {
	buf->count = 0;
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
		//die("failed to resolve local socket address (err=%d)",err);
	}
	
	//Buat socket
	int fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if (fd==-1) {
		//die("%s",strerror(errno));
	}
	
	//Bind ip ke socket
	if (bind(fd,res->ai_addr,res->ai_addrlen)==-1) {
		//die("%s",strerror(errno));
	}
	
	freeaddrinfo(res);
	
	//Receive message
	char bridge[128];
	int recvlen;
	struct sockaddr_storage src_addr;
	init_buf(&buf);
	socklen_t src_addr_len=sizeof(src_addr);
	int i = 0;
	int a;
	for(;;) {
		i++;
		recvlen = recvfrom(fd,bridge,sizeof(bridge),0,(struct sockaddr*)&src_addr,&src_addr_len);
		if (recvlen > 0) {
			bridge[recvlen] = 0;
			printf("received message %d: \"%c\" (%d bytes)\n", i, bridge[0], recvlen);
			rcvchar(&buf, bridge[0]);
		}
		while(buf.count != 0) {
			q_get(&buf);
		}
	}

	return 0;
}
