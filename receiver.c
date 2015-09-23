#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

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
	char buffer[549];
	int recvlen;
	struct sockaddr_storage src_addr;
	socklen_t src_addr_len=sizeof(src_addr);
	int i = 0;
	for(;;) {
		i++;
		recvlen = recvfrom(fd,buffer,sizeof(buffer),0,(struct sockaddr*)&src_addr,&src_addr_len);
		if (recvlen > 0) {
			buffer[recvlen] = 0;
			printf("received message %d: \"%c\" (%d bytes)\n", i, buffer[0], recvlen);
		}
	}

	return 0;
}
