#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <fcntl.h>
#include <errno.h>

static int h=-1,f=-1;
static struct sockaddr_in a;
static int sb=0,i=0,kk;
static unsigned char  data[1024];

int main(int argc,char** argv) {
  if(argv[1]==NULL) printf("no ip addr!\n"),exit(0); 
  if(argv[2]==NULL) printf("no file name!\n"),exit(0);
  f=open(argv[2],O_RDONLY);
  if(f==-1) printf("file %s not found or opened!\n",argv[2]),exit(0);
  h = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  bzero(&a, sizeof a); bzero(data,sizeof data); data[0]=68; data[1]=69; data[2]=70;
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = inet_addr(argv[1]); // this is address of host which I want to send the socket
  a.sin_port = htons(1026);
  for(;;) {
    kk=read(f,data,1024); if(kk<=0) break;
    sb = sendto(h, data, kk , MSG_DONTROUTE|MSG_CONFIRM , (const struct sockaddr*) &a, sizeof a);
    i++,printf("%d -> rc=%d\n",i,sb); 
  };
  close(f);
  shutdown(h,SHUT_WR);
  close(h);
  return 0;
}
