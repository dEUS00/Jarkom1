#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>

#define XON (0x11)
#define XOFF (0x13)

typedef enum { false=0, true } Boolean;
typedef unsigned char Byte;

const char* hostname;
const char* portname;
struct addrinfo hints;
struct addrinfo* res = 0;
int fd;

pthread_t xonoff_thread;

void *listen_xonoff(void *dummy)
{
	int recvlen;
	char bridge[128];
	while (true) {
		recvlen = recvfrom(fd,bridge,sizeof(bridge),0,res->ai_addr,&res->ai_addrlen);
		if (bridge[0] == XOFF)
			printf("XOFF diterima.\n");
	}
}

int main()
{
	//Inisialisasi
	hostname="127.0.0.1";
	portname="2121";
	int msglen = 13;
	char content[13] = "HelloaWorld! ";
	char buf[128];
	int i;

	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_ADDRCONFIG;
	int err=getaddrinfo(hostname,portname,&hints,&res);
	if (err!=0) {
		//die("failed to resolve remote socket address (err=%d)",err);
	}

	//Buat socket
	printf("Membuat socket untuk %s:%s...\n", hostname, portname);
	fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if (fd==-1) {
		//die("%s",strerror(errno));
	}
	
	pthread_create(&xonoff_thread, NULL, listen_xonoff, NULL);
	
	//Kirim message
	for(i = 0;i < msglen;i++) {
		buf[0] = content[i];
		printf("Mengirim byte ke-%d: '%c'\n", i+1, buf[0]);
		if (sendto(fd,buf,sizeof(buf),0,res->ai_addr,res->ai_addrlen)==-1) {
			//die("%s",strerror(errno));
		}
		sleep(1);
	}

	close(fd);
	
	return 0;
}
