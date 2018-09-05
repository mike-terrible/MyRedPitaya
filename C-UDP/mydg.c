#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <fcntl.h>
#include <errno.h>

static int h=-1;
static struct sockaddr_in a;
static int sb=0,i=0;
static unsigned char  data[1024];

int main(int argc,char** argv) {
  if(argv[1]==NULL) printf("no ip addr!\n"),exit(0); 
  h = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  bzero(&a, sizeof a); bzero(data,sizeof data); data[0]=68; data[1]=69; data[2]=70;
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = inet_addr(argv[1]); // this is address of host which I want to send the socket
  a.sin_port = htons(1026);
  for(i=0; i<128; i++) {
    data[i]=(i+65) % 127;
    sb = sendto(h, data, 1024 , MSG_DONTROUTE|MSG_CONFIRM , (const struct sockaddr*) &a, sizeof a);
    printf("%d -> rc=%d\n",i,sb);
  };
  shutdown(h,SHUT_WR);
  close(h);
  return 0;
}
