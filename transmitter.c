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
	const char* hostname="127.0.0.1";
	const char* portname="2121";
	struct addrinfo hints;
	char content[12] = "Hello World!";
	char buf[12];
	int i;

	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_ADDRCONFIG;
	struct addrinfo* res=0;
	int err=getaddrinfo(hostname,portname,&hints,&res);
	if (err!=0) {
		//die("failed to resolve remote socket address (err=%d)",err);
	}

	//Buat socket
	printf("Membuat socket untuk %s:%s...\n", hostname, portname);
	int fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if (fd==-1) {
		//die("%s",strerror(errno));
	}

	//Kirim message
	for(i = 0;i < 12;i++) {
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
