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
Boolean xonoff = true;

pthread_t xonoff_thread;

//Prosedur untuk thread anak
void *listen_xonoff(void *dummy)
{
	int recvlen;
	char bridge[128];
	while (true) {
		recvlen = recvfrom(fd,bridge,sizeof(bridge),0,res->ai_addr,&res->ai_addrlen);
		if (recvlen > 0) {
			if (bridge[0] == XOFF) {
				printf("XOFF diterima.\n");
				xonoff = false;
			}
			else if (bridge[0] == XON) {
				printf("XON diterima.\n");
				xonoff = true;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	//Inisialisasi
	hostname=argv[1];
	portname=argv[2];
	FILE *input = fopen(argv[3], "r");
	char buf[128];
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_ADDRCONFIG;
	int err=getaddrinfo(hostname,portname,&hints,&res);

	//Buat socket
	printf("Membuat socket untuk %s:%s...\n", hostname, portname);
	fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	
	//Buat thread anak
	pthread_create(&xonoff_thread, NULL, listen_xonoff, NULL);
	
	//Kirim message & looping file input
	int i = 0;
	char c = fgetc(input);
	while (c != EOF) {
		if (c == '\n')
			c = fgetc(input);
		else if (xonoff == false) {
			printf("Menunggu XON...\n");
			sleep(1);
		}
		else if (xonoff == true) {
			buf[0] = c;
			printf("Mengirim byte ke-%d: '%c'\n", i+1, buf[0]);
			if (sendto(fd,buf,sizeof(buf),0,res->ai_addr,res->ai_addrlen) == -1)
				return -1;
			i++;
			c = fgetc(input);
			sleep(1);
		}
	}
	
	fclose(input);
	close(fd);
		
	return 0;
}
