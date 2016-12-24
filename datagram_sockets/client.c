#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#define IP "127.0.0.1"
#define BUFLEN 1300
#define NPACK 50
#define PORT 51720
/* diep(), #includes and #defines like in the server */

void diep(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
	//parsing the command line arguments
	int P=atoi(argv[1]);
    int T=atoi(argv[2]);
    if(T%2)
    {
		printf("Value of T must be even");
		exit(0);
	}
	
	/* Socket connection*/
    struct sockaddr_in si_other, si_me;
	int s, i, slen=sizeof(si_other);
	char buf[P];

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
		diep("socket");//failed to connect socket
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
	if (inet_aton(argv[4], &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, &si_me, sizeof(si_me))==-1)
        diep("bind");
    
    //open file to write RTT values
    FILE *file;
    file=fopen(argv[3],"a");
    
    //50 datagrams to be sent
	for (i=0; i<NPACK; i++) {
		//get start time and build packet
		struct timeval tv;
		gettimeofday(&tv,NULL);
    
		int rc = T;
		int secs,usecs,seq;
		while(rc > 0){
			
			//fill buffer to be sent
			sprintf(buf, "%d %d %d %d\n", i, tv.tv_sec, tv.tv_usec, rc);
			//send message to server
			if (sendto(s, buf, P , 0, &si_other, slen)==-1)
				diep("sendto()");
			
			/*Receive message from server*/
		    //set timer to detect packet loss
		  
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(s,&read_fds);
			struct timeval timer;
			timer.tv_sec=0;
			timer.tv_usec=200000;
			try:
			//wait till timer expires for socket to be ready to read
			select(s+1, &read_fds, NULL, NULL, &timer);
			if(!FD_ISSET(s, &read_fds))
			  break;//packet loss, send next packet
		
			//if no packet loss, receive message in new struct
			if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
				diep("recvfrom()");
			
			//parse buffer
			int j=0;
			seq=0;
			for(;j<P;j++){
				if(buf[j]==' ')
					break;
				seq = seq*10+(buf[j]-'0');
			}
			j++;
			secs = 0;
			for(;j<P;j++){
				if(buf[j]==' ')
					break;
				secs = secs*10+(buf[j]-'0');
			}
			j++;
			usecs = 0;
			for(;j<P;j++){
				if(buf[j]==' ')
					break;
				usecs = usecs*10+(buf[j]-'0');
			}
			j++;
			rc = 0;
			for(;buf[j]!='\n';j++){
				if(buf[j]==' ')
					break;
				rc = rc*10+(buf[j]-'0');
			}
			//check that this is the packet expected in this loop by sequence number
			if(seq!=i)
				goto try;
			//decrement RC value
			rc--;
			
		}
		if(rc==0)	//if normal termination
		{
			printf("RC over");
			struct timeval t1;
			//get time now
			gettimeofday(&t1,0);
			long RTT = (t1.tv_sec-secs)*1000000 + t1.tv_usec-usecs;
			//write sequence number and RTT to file
			fprintf(file,"%ld\n",RTT);
			
		}
		else      //if termination due to packet loss
			continue;
    }
   close(s);
   return 0;
}
