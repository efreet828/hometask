// продолжение следует, открыл для себя наличие bpf  SO_ATTACH_FILTER.

/*
 пример ввода данных для выполнения программы 
 gcc task3.c -o task3 -luv
 sudo ./task3 eth0 eth1 3512 2153 192.168.1.54 192.168.1.175
 sudo ./task3 (физ порт приема) (физ порт отправки) (порт source addr) (порт destination addr) (source ip) (destination ip) 
 для облегчения работоспособности выводится лог в  файл в  котором пишется принятый payload и инвертированный.
 внимание!!!
 программа хорошо и стабильно работает если порт приема!= порт отправки(eth0 eth0  по вполне логичным причинам,
 т.к. пока нет фильтра на самим собой отправленные пакетыи  ответы на них, работать  будет через одно место)
 фильтр можно поставить след вида:
if(адрес отправки равен нашему адресу и адрес приема равен адресу отправки) {return;}
if(адрес приема равен нашему адресу и адрес отправки равен адресу отправки) {return;}

рабочий фильтр выглядит так:

if(iph->daddr==dst_addr && iph->saddr==src_addr) {return;}
if(iph->daddr==src_addr && iph->saddr==dst_addr) {return;}

https://gist.github.com/oro350/8269805 - интрересный вариант.

при использовании разных портов строчки из void idle_callback(uv_idle_t* handle):
unsigned int usecs=1000000;
        usleep(usecs);
	
	можно смело убирать!!!
  */

//
#include <stdio.h>
#include <net/if.h>
#include <assert.h>
#include <string.h>
#include "uv.h"
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
void To_ssend(unsigned char *, int );
void obrr(unsigned char *, int );
unsigned short csum(unsigned char *, int );
//unsigned char* part1;
unsigned char buffer[65536];
struct sockaddr_in addr;
//struct sockaddr saddr;
int saddr_size , data_size, sock_raw, a, b;
unsigned char part[65536];
u_int32_t src_addr, dst_addr;
u_int16_t src_port, dst_port;
//int sock_raw;
char *port, *port1;
int suka;
FILE *logfile;
struct sockaddr_in source, dest;
int i,j;
uv_loop_t *loop, *loop1;
typedef struct {
  uv_async_t async;
  uv_mutex_t mutex;
  uv_thread_t thread;
  uv_cond_t cond;
} thread_comm_t;
thread_comm_t comm;

unsigned short csum(unsigned char *buf, int nwords){
	unsigned long sum;
	for(sum=0; nwords>0; nwords--)
		sum += *buf++;
	sum = (sum >> 16) + (sum &0xffff);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
}

void async_cb(uv_async_t* async) {
  printf("inverted \n" );

  uv_mutex_lock(&comm.mutex);

  obrr(buffer, data_size);

  uv_cond_signal(&comm.cond);

 uv_mutex_unlock(&comm.mutex);




    }

void idle_callback(uv_idle_t* handle) {

        struct sockaddr saddr;
        saddr_size = sizeof saddr;


        data_size = recvfrom(sock_raw , buffer , 65536 , 0 , &saddr , (socklen_t*)&saddr_size);
        if(data_size <0 )
        {
          printf("Recvfrom error , failed to get packets\n");
          return;
        }
        printf("idling...\n");
        uv_async_send(&comm.async);
        uv_cond_wait(&comm.cond, &comm.mutex);
        To_ssend(buffer, data_size);
        strcpy(buffer,"");
        strcpy(part,"");
        unsigned int usecs=1000000;
        usleep(usecs);




      }

void thread_cb(void* arg) {
        loop1 = uv_loop_new();
        uv_loop_init(loop1);
        uv_idle_t idle;
        uv_idle_init(loop1, &idle);
        logfile=fopen("log.txt","w");
      	if(logfile==NULL)
      	{
      		printf("Unable to create log.txt file.");
      	}
      	int raaa;
      	raaa = socket( AF_PACKET , SOCK_RAW ,  htons(ETH_P_ALL)) ;
        if(setsockopt(raaa , SOL_SOCKET , SO_BINDTODEVICE , port, sizeof(port))<0)
      	{
        perror("Server-setsockopt() error for SO_BINDTODEVICE");
      		perror("setsockopt() error");
      		exit(-1);
      	}
      	else
      		printf("rcv setsockopt() is OK.\n");
        printf("%i\n", sock_raw );

      	if(raaa < 0)
      	{
      		//Print the error with proper message
      		perror("Socket Error");
      		return;
      	}
        sock_raw=raaa;
      //  printf("Starting...\n");
        //saddr_size = sizeof saddr;

        uv_idle_start(&idle, idle_callback);
        uv_run(loop1, UV_RUN_DEFAULT);
        //close(raaa);

      	}


void To_ssend(unsigned char *Buffer, int data_size){

  //Create a raw socket of type IPPROTO
	int s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);

	if(s == -1)
	{
		//socket creation failed, may be because of non-root privileges
		perror("Failed to create raw socket");
		exit(1);
	}

	//Datagram to represent the packet
	unsigned char datagram[65536] ,  *data;

	//zero out the packet buffer
	memset (datagram, 0, 65536);

	//IP header
	struct iphdr *iph = (struct iphdr *) datagram;

	//UDP header
	struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct ip));

	struct sockaddr_in sin;
  data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
  int nach=sizeof(struct iphdr) + sizeof(struct udphdr);
  int con=j+ sizeof(struct iphdr) + sizeof(struct udphdr);
  fprintf(logfile, "***************************\n" );
  for(i=nach; i<con; i++) {


    datagram[i]=part[i-nach];
    fprintf(logfile, " %02X", datagram[i]);
  }



	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = dst_addr;

	//Fill in the IP Header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof (struct iphdr) + sizeof (struct udphdr) + strlen(data);
	iph->id = htonl (54321);
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_UDP;
	iph->check = csum((unsigned char*)datagram, iph->tot_len);
	iph->saddr = src_addr;
	iph->daddr = sin.sin_addr.s_addr;

	//Ip checksum
	iph->check = csum ((unsigned char*) datagram, iph->tot_len);

	//UDP header
	udph->source = src_port;
	udph->dest = dst_port;
	udph->len = htons(8 + strlen(data));
	udph->check = 0;

  if(setsockopt(s , SOL_SOCKET , SO_BINDTODEVICE , port1, sizeof(port1))<0)
  {

    perror("setsockopt() error");
    exit(-1);
  }
  else
    printf("send setsockopt() is OK.\n");



	{

		if (sendto (s, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		{
			perror("sendto failed");
		}
				else
		{
			printf ("Packet Send. Length : %d \n" , iph->tot_len);
		}
	}
  close(s);
	

}

void thread_exit(void* arg) {
  char qw = getchar();

 if (qw == 'e') {
     exit(-1);
   }
}



void obrr(unsigned char *data, int data_size){


  struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
    unsigned short iphdrlen;
if (iph->protocol=17) {


    // printf("o1 \n" );
  iphdrlen = iph->ihl*4;

  struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen  + sizeof(struct ethhdr));
  memset(&source, 0, sizeof(source));
  source.sin_addr.s_addr = iph->saddr;

  memset(&dest, 0, sizeof(dest));
  dest.sin_addr.s_addr = iph->daddr;
  int header_size =  sizeof(struct ethhdr) + iphdrlen + sizeof udph;
  fprintf(logfile , " ********************************************************************************************");
  fprintf(logfile , "   |-Checksum1 : %d\n",ntohs(iph->check));
  //csum((unsigned short *)buffer, packet_len)
  //fprintf(logfile , "   |-Checksum2 : %d\n",csum(buffer, data_size));
      // printf("o2 \n" );
      //fprintf(logfile , "header_size: %i \n", header_size);
      //fprintf(logfile , "strlen(buffer) %li\n", strlen(buffer));  // data_size
      //fprintf(logfile , "data_size %i\n", data_size);
      fprintf(logfile , "   |-Source Port      : %d\n" , ntohs(udph->source));
    	fprintf(logfile , "   |-Destination Port : %d\n" , ntohs(udph->dest));
    //  printf("o3 \n" );
      fprintf(logfile , "   |-Source IP        : %s\n",inet_ntoa(dest.sin_addr));
    	fprintf(logfile , "   |-Destination IP   : %s\n",inet_ntoa(source.sin_addr));
      printf("   |-Source IP        : %s\n",inet_ntoa(dest.sin_addr));
      printf( "   |-Destination IP   : %s\n",inet_ntoa(source.sin_addr));
      //fprintf(logfile , " \n  |-iphdrlen   : %i\n", iphdrlen);
      //fprintf(logfile , " \n  |-sizeof(struct ethhdr)   : %li\n", sizeof(struct ethhdr));
      //fprintf(logfile , " \n  |-sizeof udph   : %li\n", sizeof udph);
    //  fprintf(logfile , "   |-Source Port      : %d\n" , ntohs(udph->source));
    //	fprintf(logfile , "   |-Destination Port : %d\n" , ntohs(udph->dest));
 a = ntohs(udph->source);
 b = ntohs(udph->dest);
 for(i=header_size ; i < data_size ; i++)
 {    fprintf(logfile , " %02X", buffer[i]);

}

j=data_size-header_size;
//fprintf(logfile, " \n ********j %i********************  \n", j);

      for(i=header_size ; i < data_size ; i++)
      { unsigned char c;
        int len;
        len=data_size-header_size;
        part[len-1-i+header_size]=buffer[i];

      }


      //fprintf(logfile , "\n ........................\n");

    /*  for(i=0 ; i < data_size-header_size;  i++)
      {    fprintf(logfile , " %02X", part[i]);
       //part[i]=buffer[i];
     }*/


    }
  }

int main(int argc, char *argv[]) {
    //port=argv[1];
    char *p;
    port =  argv[1];
    port1 = argv[2];
    src_port = atoi(argv[3]);
    dst_port = atoi(argv[4]);
    src_addr= inet_addr(argv[5]);
    dst_addr= inet_addr(argv[6]);


    //port1=argv[2];
    //printf("%s\n", argv[1] );
    printf("%s\n", port );

    int r;
    //loop = uv_default_loop();
    //uv_loop_init(loop);

    //memset(&comm, 0, sizeof(comm));
    r = uv_thread_create(&comm.thread, thread_cb, (void*) &comm);
    assert(r == 0);
    r = uv_thread_create(&comm.thread, thread_exit, (void*) &comm);
    assert(r == 0);
    r = uv_async_init(uv_default_loop(), &comm.async, async_cb);
   assert(r == 0);
   //comm.async.data = (void*) &comm;
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);


  }
