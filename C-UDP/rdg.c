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
  bzero(&a, sizeof a); bzero(data,sizeof data);
  a.sin_family = AF_INET;
  a.sin_addr.s_addr = inet_addr(argv[1]); 
  a.sin_port = htons(1025);
  if ( bind(h, (const struct sockaddr *)&a, sizeof(a)) < 0 ) printf("bind failed\n"),exit(0);
  printf("waiting...\n");
  for(;;) {
    sb = recv(h, data, 1024 , MSG_WAITALL);
    i++;
    if(sb<0) { printf("%d <== rc=%d\n",i,sb); }
    else printf("%d <== rc=%d,%s\n",i,sb,data);
  };
  shutdown(h,SHUT_RD);
  close(h);
  return 0;
}
