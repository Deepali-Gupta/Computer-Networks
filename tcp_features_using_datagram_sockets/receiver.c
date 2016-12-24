#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFLEN 1300
#define PORT 9930

//Function to return errors
void diep(char *s){
  perror(s);
  exit(1);
}

int main(int argc, char **argv){

    //Socket initialization
    struct sockaddr_in si_me, si_other;
    int s, i, slen = sizeof(si_other);
    char buf[BUFLEN];
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
    // fcntl(s, F_SETFL, O_NONBLOCK);
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Binding
    if (bind(s, &si_me, sizeof(si_me))==-1)
        diep("bind");
    printf("apple\n\n");
    int ack = 0;
    int start_seq, pkt_size;
    //Start receiving packets
    while(1)
    {
        if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
            diep("recvfrom()");
    //    printf("Received packet from %s:%d ::: Data: %s\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
        
        //Parsing the packet string to extract information
        start_seq = 0;
        int i;
        for(i=0;i<BUFLEN;i++) {
            if(buf[i]==' ')
                break;
            start_seq = start_seq*10+(buf[i]-48);
        }
        i++;
        pkt_size = 0;
        for(;i<BUFLEN;i++) {
            if(buf[i]==' ')
                break;
            pkt_size = pkt_size*10+(buf[i]-48);
        }
        i++;
        //update ack
        if(start_seq == ack) {
            ack =start_seq + pkt_size;
        }
        
        //for debugging sake
        printf("start_seq=%d pkt_size=%d ack=%d\n", start_seq, pkt_size, ack);

        //Rewrite information
        sprintf(buf, "%d \n", ack);            
        
        //Send packet back to client
        if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
            diep("sendto()");
        printf("sent successfully\n");
        
    }

    //close socket
    close(s);
    return 0;
}