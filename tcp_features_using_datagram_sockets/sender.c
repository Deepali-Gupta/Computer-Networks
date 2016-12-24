#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MSS 1000


void diep(char *s){
  perror(s);
  exit(1);
}


//To seed the random number generator
int seed(){
  struct timeval now;
  gettimeofday(&now, NULL);
  return now.tv_usec;
}

// //To compute difference between current time and timestamp on packet
float time_diff(int tvsec, int tvusec){
  struct timeval now;
  gettimeofday(&now, NULL);
  int t1 = now.tv_sec;
  int t2 = now.tv_usec;
  float ans = 0;
  if(t2 >= tvusec)
    ans = ans + ((float)t2-(float)tvusec)/1000000.0;
  else{
    t1 = t1 - 1;
    ans = ans + ((float)t2 + 1000000.0 - (float)tvusec)/1000000.0;
  }
  int diff = t1 - tvsec;
  ans = ans + diff;
  return ans;
}



//To check if timer of any packet has expired
int timer_expired(struct timeval timer_time)
{
  float p = time_diff(timer_time.tv_sec, timer_time.tv_usec);
  if (p>1){
    //printf("\n\n\n\nTimer Expired\n\n\n\n");
    return 1;
  }
  return 0;
}

int main(int argc, char **argv){
  int PORT = 9930;
  char* SRV_IP = "192.168.0.107";
  int l = 0;
  
  //Reading Command Line Arguments
  if(argc > 1){
    SRV_IP = argv[1];
    PORT = atoi(argv[2]);
    if(argc>3){
      l = atoi(argv[3]);
    }
  }
  //Socket Initialization
  struct sockaddr_in si_other, si_me;
  int s, i, slen=sizeof(si_other);
  char buf[MSS];

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    diep("socket");

  //Make socket Non-binding
  fcntl(s, F_SETFL, O_NONBLOCK);
  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(PORT);
  if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(PORT);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  
  //Binding
  if (bind(s, &si_me, sizeof(si_me))==-1)
    diep("bind");

  
  //Record Start time of experiment
  struct timeval start_time;
  gettimeofday(&start_time, NULL);
  

  float te;

  //Start sending packets
  int timer_seq[10000];   //sequence number of packets in timer array
  struct timeval timer_time[10000];  //expirty time of corresponding timer_seq packet
  int last_filled=-1;     //last entry filled in timer_seq and timer_time
  int last_ack=0;         //currently acked from server
  int seq=0;
  int window=MSS;

  //Send until complete 100000 bytes are sent
  while (last_ack <= 99999)
  {
    //Check if a packet has been received
    if (recvfrom(s, buf, MSS, 0, &si_other, &slen)!=-1)
    {
      
      //Parse the received packet
      int p=0;
      int temp_ack = 0;
      int i;
      for(i=0;i<MSS;i++) {
          if(buf[i]==' ')
              break;
          temp_ack = temp_ack*10+(buf[i]-48);
      }

      //Check if the recevied ACK is better than the existing highest ACK
      if (temp_ack > last_ack)
      {
        last_ack = temp_ack;
        float p1 = MSS*MSS/(float)window;
        window = window + floor(p1);

        //Remove all timers less than last ack.
        int j;
        int new;
        for (j=last_filled;j>=0;j--)
        {
          if (timer_seq[j] + MSS - 1 < last_ack)
          {
            new=j+1;
            break;
          }
        }
        for (j=new;j<=last_filled;j++)
        {
          timer_seq[j-new] = timer_seq[j];
          timer_time[j-new] = timer_time[j];
        }
        last_filled = last_filled-new;
        //printf("Last Filled %d\n",last_filled);
      }

      //Note time from start of experiment
      te = time_diff(start_time.tv_sec, start_time.tv_usec);
      printf("Received %d W = %d Time = %f\n",temp_ack, window, te);
    }


    if (last_filled != -1)
    {
      //Check if any of the timers expired
      if (timer_expired(timer_time[0]))
      {
        //Reset timer array and start from last_ack
        printf("\n\n\nTimer Expired of %d\n\n\n", timer_seq[0]);
        last_filled=-1;
        window=MSS;
        seq = last_ack;
      }
    }

    
    if (seq + MSS - last_ack <= window && seq<99999)
    {
      //Send next packet
      char buf[MSS];
      char abc[10];
      snprintf(abc,10,"%d",seq);
      strcpy(buf,abc);
      strcat(buf," ");
      snprintf(abc,10,"%d",MSS);
      strcat(buf,abc);
      strcat(buf," ");

      //Inducing Random Packet drop
      if(l){ 
        srand(seed());
        int w = rand()%20;
        if(w>=1){
          if (sendto(s, buf, MSS, 0, &si_other, slen)==-1)
              diep("sendto()");
        }
        else
          printf("\n\n\n\nInduced Drop\n\n\n");
      }
      else{
        if (sendto(s, buf, MSS, 0, &si_other, slen)==-1)
              diep("sendto()");
      }
      te = time_diff(start_time.tv_sec, start_time.tv_usec);
      printf("Sent %d W = %d Time = %f \n",seq, window, te);
      
      //Set timer of last packet sent
      struct timeval tv;
      gettimeofday(&tv,NULL);
      timer_seq[last_filled+1] = seq;
      timer_time[last_filled+1].tv_sec = tv.tv_sec;
      timer_time[last_filled+1].tv_usec = tv.tv_usec;
      last_filled++;
      seq = seq + MSS;

    }

  }
  close(s);
  return 0;
}