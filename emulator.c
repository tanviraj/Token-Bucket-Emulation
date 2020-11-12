#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cs402.h"
#include <math.h>
#include "my402list.h"
#include "defs.h"

/*void *child(void *arg)
{
  if (*(int*)arg > 2) pthread_exit((void*)1);
  return((void*)2);
}*/
int B=10;
int n=20;
int P=3;
int j=0;
int arrivalthreadexit=0;
int tokenthreadexit=0;
int server1threadexit=0;
int server2threadexit=0;
double droppktcount=0;
double droptokencount=0;
double finaltokencount=0;
int num_serv=0;
double lambda=1;
int mode=1;
double tokenintertime=0;
double packetintertime=0;
double servicetime=0;
double servicesleep=0;
//long packetsleeptime=0;
//long tokensleeptime=0;
double temptimebeforesleep=0;
double temptokentimebeforesleep=0;
double temppacketarrivaltime=0;
double temptokenarrivaltime=0;
double temppq1outstamp=0;
double temptq1outstamp=0;
double temppq1instamp=0;
double temptq2instamp=0;
double temppq2instamp=0;
double temppq2outstamp=0;
double tempservicestart=0;
double interarrivaltime=0;
double totalinterarrivaltime=0;
double timeinq1=0;
double timeinq2=0;
double tstart=0;
double tempemulationend=0;
double total_emutime=0;
double pa=0;
double system_time=0;
double tempserviceend=0;
double st=0;
double s=0;
double q1_timespentsum=0;
double q2_timespentsum=0;
double s1_timespentsum=0;
double s2_timespentsum=0;
double sum_timeinsystem=0;
double sq_systime=0;
double sum_sqsystime=0;
double sq_avg=0;
double variance=0;
double std_dev=0;
double rate=1.5;
double mu=0.35;
char filename[50];
char cl1[50];
char cl2[50];
char cl3[50];
double emulationstarttime=0.0;
FILE *fp=NULL;
int tokencount=0;
My402List packetlist;
My402List serverlist;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t PQueue = PTHREAD_COND_INITIALIZER;
struct timeval packetarrivaltime;
struct timeval pq1instamp;
struct timeval pq1outstamp;
struct timeval pq2instamp;
struct timeval pq2outtimestamp;
struct timeval tq1outstamp;
struct timeval tq2instamp;
struct timeval timebeforesleep;
struct timeval tokentimebeforesleep;
struct timeval tokenarrivaltime;
struct timeval temulationstart;
struct timeval temulationend;
struct timeval servicestartstamp;
struct timeval serviceendstamp;
double packetprearrivaltime;
//MyPacket *pkt=NULL;

typedef struct packet
{
int packetnum;
int ntokens;
double servicetime;
struct timeval packetarrivaltime;
struct timeval pq1instamp;
struct timeval pq1outstamp;
struct timeval pq2instamp;
struct timeval pq2outstamp;
}
MyPacket;

MyPacket *pkt=NULL;

struct timeval gettime()
{
struct timeval tv;
gettimeofday(&tv,NULL);
//unsigned long time_in_micro=1000000*tv.tv_sec+tv.tv_usec;
return tv;
}

void *packetqueue(void *arg)
{  
   //pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
   //My402List packetlist;
   My402ListElem *elem= NULL;
   double packetsleeptime=0;
   
   char buf1[1024];
   char *ptr1;
   char *ptr2;

   //Note: Change this to timeval later

   for(int i=0;i<n;i++){
   if(mode==0) {
      if(fgets(buf1, sizeof(buf1), fp)!=NULL) {
    	sscanf(buf1,"%s %s %s",cl1,cl2,cl3);
    	packetintertime=strtoul(cl1,&ptr1,10)*1000;
    	P=atoi(cl2);
    	servicetime=strtoul(cl3,&ptr2,10)*1000;
        }
        }
	//Get a timestamp before sleeping
        //{
        timebeforesleep=gettime();
        temptimebeforesleep=timebeforesleep.tv_sec*1000000+timebeforesleep.tv_usec;
       	
        packetsleeptime=packetintertime-(temptimebeforesleep-temppacketarrivaltime);
        //}
        
	//printf("Before sleep in arrival\n");
	usleep(packetsleeptime);
            packetprearrivaltime=temppacketarrivaltime;
        //Get t1 timestamp here(T1)---->Packet arrival time
	    packetarrivaltime=gettime();
            
        temppacketarrivaltime=(packetarrivaltime.tv_sec*1000000+packetarrivaltime.tv_usec);
        interarrivaltime=(temppacketarrivaltime-tstart)-(packetprearrivaltime-tstart);
        totalinterarrivaltime=totalinterarrivaltime+interarrivaltime;


        //Check for packet drop

        if(P>B)
        {
        fprintf(stdout,"%012.3lfms: p%d arrives, needs %d tokens, inter-arrival time = %0.8fms, dropped\n",(double)((temppacketarrivaltime-tstart)/1000),i+1,P,(double)((interarrivaltime)/1000));
        droppktcount=droppktcount+1;
        continue;
        }
        //printf("Before printf\n");
        fprintf(stdout,"%012.3lfms: p%d arrives, needs %d tokens, inter-arrival time = %0.8fms\n",(double)((temppacketarrivaltime-tstart)/1000),i+1,P,(double)((interarrivaltime)/1000));

        //printf("After printf\n");

        //Create Packet
        //packet = NewPacket(i+1,P,servicetime,packetarrivaltime);
         MyPacket *pkt=(MyPacket *)malloc(sizeof(MyPacket));
         pkt->packetnum=i+1;
         pkt->ntokens=P;
         pkt->servicetime=servicetime;
         pkt->packetarrivaltime=packetarrivaltime;


        //Lock Mutex
        pthread_mutex_lock(&m);
 
        if(My402ListEmpty(&packetlist)) {

        My402ListAppend(&packetlist,(void *)pkt);
        //Get p1 enters Q1 timestamp here(T2)
        pq1instamp=gettime();
        //Added this timestamp to packet structures
        pkt->pq1instamp=pq1instamp;

        temppq1instamp=pq1instamp.tv_sec*1000000+pq1instamp.tv_usec;
        fprintf(stdout,"%012.3lfms: p%d enters Q1\n",(double)((temppq1instamp-tstart)/1000),i+1);

        

        //Move from Q1 to Q2 if enough tokens
        if(tokencount >= pkt->ntokens)
        {
        elem=My402ListFirst(&packetlist);
        My402ListUnlink(&packetlist, elem);
        //Get p1 leaves Q1 timestamp here(T3)
        pq1outstamp=gettime();
        //Add this timestamp to packet structure
        pkt->pq1outstamp=pq1outstamp;
        
        temppq1outstamp=pq1outstamp.tv_sec*1000000+pq1outstamp.tv_usec;
        timeinq1=(temppq1outstamp-tstart)-(temppq1instamp-tstart);
        tokencount=tokencount-(pkt->ntokens);
      //variable for calculating avg no. of packets in q1
       q1_timespentsum=q1_timespentsum+timeinq1;

        fprintf(stdout,"%012.3lfms: p%d leaves Q1, time in Q1 = %0.8f, token bucket now has %d tokens\n",(double)((temppq1outstamp-tstart)/1000),i+1,(double)((timeinq1)/1000),tokencount);
        
        if(My402ListEmpty(&serverlist))
        {
        //Signal queue not empty condition before appending
        //printf("Signalling from pkt\n");
        pthread_cond_broadcast(&PQueue);
        //printf("Signalling from pkt\n");
        }

        My402ListAppend(&serverlist,(void *)pkt);
        //Get p1 enters Q2 timestamp(T4)
	pq2instamp=gettime();
        //Add this timestamp to pkt
        pkt->pq2instamp=pq2instamp;
        temppq2instamp=pq2instamp.tv_sec*1000000+pq2instamp.tv_usec;
        fprintf(stdout,"%012.3lfms: p%d enters Q2\n",(double)((temppq2instamp-tstart)/1000),i+1);
     
        }
        }
        else if(!My402ListEmpty(&packetlist))
        {
        My402ListAppend(&packetlist,pkt);
        //Get p1 enters Q1 timestamp here(T2)
        pq1instamp=gettime();
        //Added this timestamp to packet structures
        pkt->pq1instamp=pq1instamp;


        temppq1instamp=pq1instamp.tv_sec*1000000+pq1instamp.tv_usec;
        fprintf(stdout,"%012.3lfms: p%d enters Q1\n",(double)((temppq1instamp-tstart)/1000),i+1);

        }
	//Unlock Mutex
	pthread_mutex_unlock(&m);
        }
        
	//Check if we have to add mutex lock
        arrivalthreadexit=1;

        pthread_exit((void *)1);
}

void *tokenqueue(void *arg)
{
  My402ListElem *elem= NULL;
  double tokensleeptime=0;
  for( ; ; )
  {
   j=j+1;
        //printf("Entered else\n");
        tokentimebeforesleep=gettime();

        temptokentimebeforesleep=tokentimebeforesleep.tv_sec*1000000+tokentimebeforesleep.tv_usec;


        tokensleeptime=tokenintertime-(temptokentimebeforesleep-temptokenarrivaltime);


  //printf("Exited Else\n");
  //printf("Token sleep time:%lf\n",tokensleeptime);
  usleep(tokensleeptime);
  //printf("Woke up\n");

  //Get t1 timestamp here(T1)---->token arrival time
  tokenarrivaltime=gettime();

  temptokenarrivaltime=(tokenarrivaltime.tv_sec*1000000+tokenarrivaltime.tv_usec);


  //lock mutex
  pthread_mutex_lock(&m);

  //token arrives, increment tokencount
  tokencount=tokencount+1;
  

  //Dropping token
  if(tokencount>B)
    {
    tokencount--;
    fprintf(stdout,"%012.3lfms: token t%d arrives, token dropped\n",(double)((temptokenarrivaltime-tstart)/1000),j);
    droptokencount=droptokencount+1;
    pthread_mutex_unlock(&m);
    }
  else
    {
    //fprintf(stdout,"token arrival time just before printing: %lf\n",temptokenarrivaltime);
    fprintf(stdout,"%012.3lfms: token t%d arrives, token bucket now has %d token\n",(double)((temptokenarrivaltime-tstart)/1000),j,tokencount);
    //}

   //Check if q1 is not empty
   if(!My402ListEmpty(&packetlist))
   {
     elem=My402ListFirst(&packetlist);
     MyPacket *p=(MyPacket *)elem->obj;
     if(p->ntokens<=tokencount)
     {
     tokencount=tokencount-(p->ntokens);
     My402ListUnlink(&packetlist, elem);
     //timestamp q1 exit
     tq1outstamp=gettime();
     temptq1outstamp=tq1outstamp.tv_sec*1000000+tq1outstamp.tv_usec;
     p->pq1outstamp=tq1outstamp;
     //timeinq1=p->pq1outstamp-p->pq1instamp;
     temppq1instamp=p->pq1instamp.tv_sec*1000000+p->pq1instamp.tv_usec;
     timeinq1=(temptq1outstamp-tstart)-(temppq1instamp-tstart);

    //calculating sum of time spent in q1
    q1_timespentsum=q1_timespentsum+timeinq1;

     fprintf(stdout,"%012.3lfms: p%d leaves Q1, time in Q1 = %0.8fms, token bucket now has %d token\n",(double)((temptq1outstamp-tstart)/1000),p->packetnum,(double)((timeinq1)/1000),tokencount);

     //Check if q2 is empty
     if(My402ListEmpty(&serverlist))
        {
        //Signal queue not empty condition before appending
        //printf("Signalling from token\n");
        pthread_cond_broadcast(&PQueue);
        //printf("Signalling from token\n");
        }

     My402ListAppend(&serverlist,(void *)p);

     //timestamp Q2 entry
     tq2instamp=gettime();
     temptq2instamp=tq2instamp.tv_sec*1000000+tq2instamp.tv_usec;
     p->pq2instamp=tq2instamp;
     fprintf(stdout,"%012.3lfms: p%d enters Q2\n",(double)((temptq2instamp-tstart)/1000),p->packetnum);

     }
   }
     
//unlock mutex
pthread_mutex_unlock(&m);
}
//printf("Reached\n");
if((My402ListEmpty(&packetlist)) && arrivalthreadexit==1)
{
//printf("Inif\n");
tokenthreadexit=1;
pthread_exit((void *)2);
}
}
  /*if (*(int*)arg > 2) pthread_exit((void*)1);
  return((void*)2);*/
}

void *serverqueue1(void *arg)
{
  My402ListElem *elem= NULL;
  for( ; ;)
  {
  //locks mutex
  pthread_mutex_lock(&m);

  //Figure out this loop, Sleep until list becomes non-empty
  while(My402ListEmpty(&serverlist) && (tokenthreadexit==0 || arrivalthreadexit==0))
  {
  	pthread_cond_wait(&PQueue,&m);
//	printf("in while s1\n");

  }
  //  printf("after while s1\n");
  if(!My402ListEmpty(&serverlist))
  {
  //Dequeue a pkt
  //printf("in if 1\n");
  elem=My402ListFirst(&serverlist);

 MyPacket *p=(MyPacket *)elem->obj;

  My402ListUnlink(&serverlist, elem);

  //Get time stamp
  pq2outtimestamp=gettime();
//  MyPacket *p=(MyPacket *)elem->obj;

  //Adding the timestamp to the pkt
  p->pq2outstamp=pq2outtimestamp;

  temppq2outstamp=pq2outtimestamp.tv_sec*1000000+pq2outtimestamp.tv_usec;
  temppq2instamp=(p->pq2instamp).tv_sec*1000000+(p->pq2instamp).tv_usec;

  timeinq2=(temppq2outstamp-tstart)-(temppq2instamp-tstart);

  //caculating total time spent in q2 for all packets 
  q2_timespentsum=q2_timespentsum+timeinq2;

  fprintf(stdout,"%012.3lfms: p%d leaves Q2, time in Q2 = %0.8f\n",(temppq2outstamp-tstart)/1000,p->packetnum,timeinq2/1000);

  servicestartstamp=gettime();
  tempservicestart=servicestartstamp.tv_sec*1000000+servicestartstamp.tv_usec;
 fprintf(stdout,"%012.3lfms: p%d begins service at S1, requesting %0.8fms of service\n",(tempservicestart-tstart)/1000,p->packetnum,(p->servicetime)/1000);


  //Unlock Mutex
  pthread_mutex_unlock(&m);

  //Transmit the packet and Sleep
  servicesleep=p->servicetime;
  usleep(servicesleep);

//timestamp when it departs server
serviceendstamp=gettime();
tempserviceend=serviceendstamp.tv_sec*1000000+serviceendstamp.tv_usec;
//calculating service time
st=(tempserviceend-tstart)-(tempservicestart-tstart);
//calculating time in system
pa=p->packetarrivaltime.tv_sec*1000000+p->packetarrivaltime.tv_usec;
system_time=(tempserviceend-tstart)-(pa-tstart);

//variables for calculating avg pkt service time
s=s+st;   //in microsec
num_serv=num_serv+1;   //num of pakets serviced

//caculating total time spent by all packets in s1 
s1_timespentsum=s1_timespentsum+st;

//calculating sum of  pkt time spent in system
sum_timeinsystem=sum_timeinsystem+system_time;

//square of system time
sq_systime=(system_time/1000000)*(system_time/1000000);
//printf(" sq sys %lf\n",sq_systime);
sum_sqsystime=sum_sqsystime+sq_systime;
fprintf(stdout,"%012.3lfms: p%d departs from S1, service time = %0.8fms, time in system = %0.8fms\n",(tempserviceend-tstart)/1000,p->packetnum,st/1000,system_time/1000);
  }
  if(My402ListEmpty(&serverlist) && tokenthreadexit==1 && arrivalthreadexit==1)
  {
  //Server should exit

  server1threadexit=1;

  //Broadcast to 2
  pthread_cond_broadcast(&PQueue);
 pthread_mutex_unlock(&m);
  finaltokencount=(double)j;
  pthread_exit((void *)3);
  }

  }
}

void *serverqueue2(void *arg)
{
  My402ListElem *elem= NULL;
  for( ; ;)
  {
  //locks mutex
  pthread_mutex_lock(&m);

  //Figure out this loop, Sleep until list becomes non-empty
  while(My402ListEmpty(&serverlist) && (tokenthreadexit==0 || arrivalthreadexit==0))
  {
        pthread_cond_wait(&PQueue,&m);
	//printf("in while s2\n");
  }
//printf("after while s2\n");
  if(!My402ListEmpty(&serverlist))
  {
  //printf("In if 2\n");
  //Dequeue a pkt
  elem=My402ListFirst(&serverlist);
 MyPacket *p=(MyPacket *)elem->obj;
  My402ListUnlink(&serverlist, elem);

  //Get time stamp
  pq2outtimestamp=gettime();
  //Adding the timestamp to the pkt
  p->pq2outstamp=pq2outtimestamp;

  temppq2outstamp=pq2outtimestamp.tv_sec*1000000+pq2outtimestamp.tv_usec;

  temppq2instamp=(p->pq2instamp).tv_sec*1000000+(p->pq2instamp).tv_usec;

  timeinq2=(temppq2outstamp-tstart)-(temppq2instamp-tstart);

  //caculating total time spent in q2 for all packets 
  q2_timespentsum=q2_timespentsum+timeinq2;

  fprintf(stdout,"%012.3lfms: p%d leaves Q2, time in Q2 = %0.8f\n",(temppq2outstamp-tstart)/1000,p->packetnum,timeinq2/1000);
  servicestartstamp=gettime();
  tempservicestart=servicestartstamp.tv_sec*1000000+servicestartstamp.tv_usec;
  fprintf(stdout,"%012.3lfms: p%d begins service at S2, requesting %0.8fms of service\n",(tempservicestart-tstart)/1000,p->packetnum,(p->servicetime)/1000);

  //Unlock Mutex
  pthread_mutex_unlock(&m);

  //Transmit th  packet and Sleep
  servicesleep=(p->servicetime);
  usleep(servicesleep);

 //timestamp when it departs server
serviceendstamp=gettime();
tempserviceend=serviceendstamp.tv_sec*1000000+serviceendstamp.tv_usec;
//calculating service time
st=(tempserviceend-tstart)-(tempservicestart-tstart);
//calculating time in system
pa=p->packetarrivaltime.tv_sec*1000000+p->packetarrivaltime.tv_usec;
system_time=(tempserviceend-tstart)-(pa-tstart);

//variables for calculating avg pkt service time
s=s+st;   //in microsec
num_serv=num_serv+1;   //num of packets serviced

//caculating total time spent by all packets in s2 
s2_timespentsum=s2_timespentsum+st;


//calculating sum of  pkt time spent in system
sum_timeinsystem=sum_timeinsystem+system_time;


//square of system time
sq_systime=(system_time/1000000)*(system_time/1000000);
sum_sqsystime=sum_sqsystime+sq_systime;
fprintf(stdout,"%012.3lfms: p%d departs from S2, service time = %0.8fms, time in system = %0.8fms\n",(tempserviceend-tstart)/1000,p->packetnum,st/1000,system_time/1000);

  }
  if(My402ListEmpty(&serverlist) && tokenthreadexit==1 && arrivalthreadexit==1)
  {
  //Server should exit
  server2threadexit=1;

  //Broadcast to server2

  //printf("Just before broadcast2\n");
  pthread_cond_broadcast(&PQueue);

  //pthread_exit((void *)4);

 pthread_mutex_unlock(&m);
  pthread_exit((void *)4);
  }

  }
}

int main(int argc, char *argv[])
{


  //Parse command line arguments
  /* 1 -deterministic
     0 -trace-driven mode*/
  char buf[1024];

  for(int i=0;i<argc;i++)
  {
  if(strcmp(argv[i],"-t")==0)
  {
  strcpy(filename,argv[i+1]);
  mode=0;
  break;
  }
  }
  //Trace mode
  if(mode==0)
  {
  for(int i=0;i<argc;i++)
  {
  	if(strcmp(argv[i],"-r")==0) {
        rate=atof(argv[i+1]);
        if(rate<0){
        fprintf(stderr," r must be a positive real num\n");
        exit(1);
        }        
        }
	
	else if(strcmp(argv[i],"-B")==0) {
	B=atoi(argv[i+1]);
        if(B<0 || B>= 2147483647){
        fprintf(stderr," B must be a positive integer < 2147483647\n");
        exit(1);
        }

	}
  }
 
  //File processing
  fp = fopen(filename, "r");
            if (fp == NULL) {
                fprintf(stderr, "Input file %s does not exist.\n", filename);
                exit(0);}
  while(fgets(buf, sizeof(buf), fp) != NULL) {
    n=atoi(buf);
    break;
  }

//Emulation parameters
fprintf(stdout,"Emulation Parameters:\n");
fprintf(stdout,"    number to arrive = %d\n",n);
fprintf(stdout,"    r = %lf\n",rate);
fprintf(stdout,"    B = %d\n",B);
fprintf(stdout,"    tsfilename = %s\n",filename);
fprintf(stdout,"%012.3lfms: emulation begins\n",emulationstarttime);

  }

  //Deterministic mode
  else if(mode==1)
  {

  for(int i=0;i<argc;i++)
 {
  if(strcmp(argv[i],"-lambda")==0) {
  lambda=atoi(argv[i+1]);
  if(lambda<0){
	fprintf(stderr," Lambda must be a positive real num\n");
        exit(1);
  }
  }
  else if(strcmp(argv[i],"-mu")==0) {
  mu=atof(argv[i+1]);
  if(mu<0){
        fprintf(stderr," mu must be a positive real num\n");
        exit(1);
  }

  }
  else if(strcmp(argv[i],"-r")==0) {
  rate=atof(argv[i+1]);
  if(rate<0){
        fprintf(stderr," r must be a positive real num\n");
        exit(1);
  }
  }
  else if(strcmp(argv[i],"-B")==0) {
  B=atoi(argv[i+1]);
  if(B<0 || B>= 2147483647){
  	fprintf(stderr," B must be a positive integer < 2147483647\n");
        exit(1);
  }
  }
  else if(strcmp(argv[i],"-P")==0) {
  P=atoi(argv[i+1]);
  if(P<0 || P>= 2147483647){
        fprintf(stderr," P must be a positive integer < 2147483647\n");
        exit(1);
  }
  }
  else if(strcmp(argv[i],"-n")==0) {
  n=atoi(argv[i+1]);
  if(n<0 || n>= 2147483647){
        fprintf(stderr," n must be a positive integer < 2147483647\n");
        exit(1);
  }

  }
}

//Packet inter time in us
//packetintertime=(1/lambda);
if((1/lambda)>10)
  //packetintertime=10;
packetintertime=10000000;
else
packetintertime=(1/lambda)*1000000;

//Service time in us
//servicetime=(1/mu);
if((1/mu)>10)
  servicetime=100000000;
else
servicetime=(1/mu)*1000000;


//Emulation parameters
fprintf(stdout,"Emulation Parameters:\n");
fprintf(stdout,"    number to arrive = %d\n",n);
fprintf(stdout,"    lambda = %lf\n",lambda);
fprintf(stdout,"    mu = %lf\n",mu);
fprintf(stdout,"    r = %lf\n",rate);
fprintf(stdout,"    B = %d\n",B);
fprintf(stdout,"    P = %d\n",P);
fprintf(stdout,"%012.3lfms: emulation begins\n",emulationstarttime);
  }

//Tokenintertime
  //tokenintertime=1/rate;
  if((1/rate)>10) 
    //tokenintertime=10;
  tokenintertime=10000000;
  else
  tokenintertime=(long)((1/rate)*1000000);
  //printf("Token inter time is %lf\n",tokenintertime);

 // printf("%012.3lfms: emulation begins\n",emulationstarttime);

  pthread_t packetthread;
  pthread_t tokenthread;
  pthread_t serverthread1;
  pthread_t serverthread2;

  void *packet_exit_status=NULL;
  void *token_exit_status=NULL;
  void *server1_exit_status=NULL;
  void *server2_exit_status=NULL;

  //get emulation start
  temulationstart=gettime();
  tstart=temulationstart.tv_sec*1000000+temulationstart.tv_usec;
  temptokenarrivaltime=tstart;
  temppacketarrivaltime=temptokenarrivaltime;

  //Create 4 threads
  pthread_create(&packetthread, 0, packetqueue, &argc);
  pthread_create(&tokenthread, 0, tokenqueue, &argc);
  pthread_create(&serverthread1, 0, serverqueue1, &argc);
  pthread_create(&serverthread2, 0, serverqueue2, &argc);

  //Wait for all 3 threads to terminate
  pthread_join(packetthread, (void**)&packet_exit_status);
  pthread_join(tokenthread, (void**)&token_exit_status);
  pthread_join(serverthread1, (void**)&server1_exit_status);
  pthread_join(serverthread2, (void**)&server2_exit_status);

  //emulation ends
  temulationend=gettime();
  tempemulationend=temulationend.tv_sec*1000000+temulationend.tv_usec;

  // total emulation time is 
  total_emutime=tempemulationend-tstart;

  fprintf(stdout,"%012.3lfms: emulation ends\n",(tempemulationend-tstart)/1000);
  
  fprintf(stdout,"Statistics:\n\n");
  fprintf(stdout,"    average packet inter-arrival time = %lf\n",((totalinterarrivaltime/1000000)/n));
  fprintf(stdout,"    average pakets service time = %lf\n",(s/1000000)/num_serv);
  fprintf(stdout,"\n");
  fprintf(stdout,"    average number of packets in Q1 = %lf\n",(q1_timespentsum/total_emutime));
  fprintf(stdout,"    average number of packets in Q2 = %lf\n",(q2_timespentsum/total_emutime));
  fprintf(stdout,"    average number of packets at S1 = %lf\n",(s1_timespentsum/total_emutime));
  fprintf(stdout,"    average number of packets at S2 = %lf\n",(s2_timespentsum/total_emutime));
  fprintf(stdout,"\n");
  fprintf(stdout,"    average time a packet spent in system = %lf\n",(sum_timeinsystem/1000000)/num_serv);
  sq_avg=(sum_sqsystime)/num_serv;
  variance=sq_avg-(((sum_timeinsystem/1000000)/num_serv)*((sum_timeinsystem/1000000)/num_serv));
  std_dev=sqrt(variance);
  fprintf(stdout,"    standard deviation for time spent in system = %lf\n\n",std_dev);
  fprintf(stdout,"    token drop probability = %lf\n",(droptokencount/finaltokencount));
  fprintf(stdout,"    packet drop probability = %lf\n",(droppktcount/(droppktcount+num_serv)));
  
  return 0;
}
