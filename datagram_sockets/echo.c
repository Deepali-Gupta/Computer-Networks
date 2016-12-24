#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 1300
#define PORT 51720

void diep(char *s)
{
  perror(s);
  exit(1);
}

int main(void)
{
	/*Configure settings in address struct*/
    struct sockaddr_in si_me, si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);//any ip address
    if (bind(s, &si_me, sizeof(si_me))==-1)
        diep("bind");

        while(1)
        {
			//receive message from client
            int bytes = recvfrom(s, buf, BUFLEN, 0, &si_other, &slen);
            if (bytes==-1)
                diep("recvfrom()");
            printf("Received packet from %s:%d ::: Data: %s\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
            //parse buffer
            int j=0;
			int seq=0;
			for(;j<BUFLEN;j++){
				if(buf[j]==' ')
					break;
				printf("%c",seq);
				seq = seq*10+(buf[j]-'0');
			}
			j++;
			int secs = 0;
			for(;j<BUFLEN;j++){
				if(buf[j]==' ')
					break;
				secs = secs*10+(buf[j]-'0');
			}
			j++;
			int usecs = 0;
			for(;j<BUFLEN;j++){
				if(buf[j]==' ')
					break;
				usecs = usecs*10+(buf[j]-'0');
			}
			j++;
			int rc = 0;
			for(;buf[j]!='\n';j++){
				if(buf[j]==' ')
					break;
				rc = rc*10+(buf[j]-'0');
			}
			//decrement RC
			rc--;
            //reconstruct buffer
            sprintf(buf,"%d %d %d %d\n",seq,secs,usecs,rc);
            //send back to client
            if (sendto(s, buf, bytes, 0, &si_other, slen)==-1)
                diep("sendto()");
            else
                printf("RC:%d\n",rc);
            
        }

    close(s);
    return 0;
}
