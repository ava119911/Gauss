#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char**argv)
{
   int sockfd,i;
   struct sockaddr_in servaddr,cliaddr;
   char sendbuf[4096];
   char *p;

   if (argc < 3)
   {
      printf("usage:  udpcli <IP address> str1 str2 ...\n");
      return 1;
   }

   sockfd=socket(AF_INET, SOCK_DGRAM, 0);
   memset(&servaddr, 0, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=inet_addr(argv[1]);
   servaddr.sin_port=htons(10033);

   for (i = 2, p = sendbuf; i < argc; i++) {
       int len = strlen(argv[i]) + 1;
       memcpy(p, argv[i], len);
       p += len;
   }

   if (sendto(sockfd, sendbuf, p - sendbuf, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
       perror("sendto failed");
       return 1;
   }

   return 0;
}
