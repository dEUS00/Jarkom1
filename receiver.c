#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>

#define XON (0x11)
#define XOFF (0x13)
#define MAXSIZE 30
#define MINUPLIM 5

typedef enum { false=0, true } Boolean;
typedef unsigned char Byte;
const char* hostname;
const char* portname;
struct addrinfo hints;
struct addrinfo* res=0;
int fd;
struct sockaddr_storage src_addr;
socklen_t src_addr_len=sizeof(src_addr);

Byte sent_xonxoff = XON;
Boolean send_xon = false, send_xoff = false;
pthread_t consume_thread;
pthread_mutex_t lock;

typedef struct BUFFER
{
	int count;
	int head;
	int tail;
	char data[MAXSIZE];
} BUFFER;


void rcvchar(BUFFER *buf, char x) {
	if(buf->count==0) {
		buf->head = 0;
		buf->data[buf->head] = x;
		buf->count++;
	}
	else {
		buf->tail = buf->count % 30;
		buf->data[buf->tail] = x;
		buf->count++;
	}
}

void q_get(BUFFER *buf) {
	if (buf->count != 0) {
		printf("%c\n", buf->data[(buf->head) % 30]);
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

void *consume(void *dummy) {
	while(true) {
		sleep(3);
		pthread_mutex_lock(&lock);
		q_get(&buf);
		if (send_xoff == true) {
			if (buf.count < MINUPLIM) {
				char send[1];
				send[0] = XON;
				if (sendto(fd,send,sizeof(send),0,(struct sockaddr*)&src_addr,src_addr_len) != -1) {
					printf("Mengirim XON\n");
					send_xoff = false;
				}
			}
		}
		pthread_mutex_unlock(&lock);
	}
}

int main()
{
	//Inisialisasi
	hostname=0;
	portname="2121";
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_UNSPEC;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_protocol=0;
	hints.ai_flags=AI_PASSIVE|AI_ADDRCONFIG;
	int err=getaddrinfo(hostname,portname,&hints,&res);
	if (err!=0) {
		return -1;
	}
	
	//Buat socket
	fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
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
	init_buf(&buf);
	int i = 0;
	pthread_create(&consume_thread, NULL, consume, NULL);
	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
	for(;;) {
		i++;
		recvlen = recvfrom(fd,bridge,sizeof(bridge),0,(struct sockaddr*)&src_addr,&src_addr_len);
		if (recvlen > 0) {
			bridge[recvlen] = 0;
			printf("Menerima byte ke-%d: '%c'\n", i, bridge[0]);
			if (bridge[0] != 'a') {
				pthread_mutex_lock(&lock);
				rcvchar(&buf, bridge[0]);
				pthread_mutex_unlock(&lock);
				if (buf.count > MINUPLIM && send_xoff == false) {
					char send[1];
					send[0] = XOFF;
					if (sendto(fd,send,sizeof(send),0,(struct sockaddr*)&src_addr,src_addr_len) != -1) {
						printf("Mengirim XOFF\n");
						send_xoff = true;
					}
				}
			}
			else
				break;
		}
	}
	
	while(buf.count != 0)
		q_get(&buf);
	printf("\n");

	return 0;
}
