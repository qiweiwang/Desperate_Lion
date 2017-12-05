/*
 * adc.c
 */

/******************************************************************************
* Include Files                                                               *
******************************************************************************/

// Standard header files
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <i2cfunc.h>

// Driver header file
#include "prussdrv.h"
#include <pruss_intc_mapping.h>	 
#include "adc.h"

#include <sys/socket.h>
//#include <sys/type.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
/******************************************************************************
* Local Macro Declarations                                                    *
******************************************************************************/

#define PRU_NUM 	 0

#define DDR_BASEADDR     0x80000000
#define OFFSET_DDR	 0x00001008

#define OFFSET_SHAREDRAM 0
#define PRUSS1_SHARED_DATARAM    4
#define CYCLE 24000
#define UNITLENGTH 4832


//original 4800


/******************************************************************************
* Local Function Declarations                                                 *
******************************************************************************/

static int LOCAL_exampleInit ( );


/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static int mem_fd;
static void *ddrMem, *sharedMem;

static int chunk;

static unsigned int *sharedMem_int;

static char dumpTargetMemo[150000000];
static char playFrom[1500000];
FILE* outfile;
static int outtxt;

/******************************************************************************
* Global Function Definitions                                                 *
******************************************************************************/

int dumpOffset = 0;

void dump2FileTXT(void)
{
   printf("dumping data from memory to TXTfile...\n");
   if(write(outtxt, dumpTargetMemo, UNITLENGTH*CYCLE)==UNITLENGTH*CYCLE)
   {
     printf("dump to txt success!\n");
    }

}




int dump2FileCSV (void)
{       
	
	printf("dumping data from memory to file...\n");
	int x, errorCount=0;
	char val, valOld;
        for (x=0; x<UNITLENGTH*CYCLE; x++)
	{      // if ((dumpTargetMemo[x])=='a'){break;}
        	val=dumpTargetMemo[x];
                 if(x>0)
                {
                  if(val==valOld){errorCount++;
                   printf("x = %d \n",x);
                   }
                }
		fprintf(outfile, "%d\n",val);
                valOld=val;
               
	}
	printf("dumping complete, total %d byte dumped\n",x);
	return errorCount;
	
}


void dump2FileCSV2(void)
{
   printf("dumping data from memory to file...\n");
   int x;
   int k;
   char val, valbit;
   for (x=0; x<UNITLENGTH*CYCLE; x++)
   { 
      val=dumpTargetMemo[x]; 
      for(k=0; k<8; k++)
      {
        valbit=(val & 0x80)>>7;
        fprintf(outfile, "%d", valbit);
        val=val<<1;
      }
      fprintf(outfile, "\n");
    }
   printf("dumping complete!\n");

}


void merge(int offsetDump, int offsetMerge)
{ int mergeCounter, bitCounter;
  char tempByte;
  for (mergeCounter=0; mergeCounter<UNITLENGTH/8; mergeCounter++)
      {  tempByte=0;
         for(bitCounter=0; bitCounter<8; bitCounter++)
         {
           tempByte=tempByte|((dumpTargetMemo[offsetDump+mergeCounter*8+bitCounter]&(0x01))<<bitCounter);
         }
      playFrom[offsetMerge+mergeCounter]=tempByte;
      }
}


void *clientStart(void *data);
void *clientStart(void *data)
{
    int sockfd, n, total=0, n_sent=0, i=0;
    struct sockaddr_in serv_addr;
    printf("in the thread\n");
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
	printf("\nError: could not create socket\n");
    }
    memset(&serv_addr, 0x00, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);
    if(inet_pton(AF_INET, "192.168.1.3", &serv_addr.sin_addr)<=0)
    {
	printf("\nerror: inet_pton error occured\n");
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
         printf("Error: Connect Failed\n");
    }

    while(1)
    {
	//printf("dumpOffset is %d, n_sent is %d, dumpOffset-n_sent is %d\n", dumpOffset, n_sent, (dumpOffset-n_sent));
        if((dumpOffset-n_sent) >=4800)
        {
            total=0;
            i++;
	    while(total!=4800)
            {
	        n = write(sockfd, dumpTargetMemo+n_sent , 4800);
                //printf("transmitte %d bytes\n", n);
                total=total + n;
                n_sent=n_sent + n;
            }
            //printf("#%d packets sent out by ethernet\n", i);
        }
    }


   return;

}


pthread_t clientThread;

int main (void)
{
    unsigned int ret;
    int i, sockfd;
    struct sockaddr_in serv_addr;
    void *DDR_paramaddr;
    void *DDR_ackaddr;
    int fin;
    char fname_new[255];
    struct timespec  start, end;
    double duration;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   
    //pthread_create(&clientThread, NULL, clientStart, NULL);    
 
    
    //open socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)

    {
	printf("\nError: Could not create socket \n");
    }

    memset(&serv_addr, 0x00, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, "192.168.1.5", &serv_addr.sin_addr)<=0)
    {
	printf("\n Error: inet_pton error occured\n\n");
    }

    


    printf("\nINFO: Starting %s example.\r\n", "ADC");
    /* Initialize the PRU */
    prussdrv_init ();		

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }
    
    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);
    
    // Open file
     // outfile=fopen("data.txt", "w");
//      outfile=fopen("data.csv", "w");


    /* Initialize example */
    printf("\tINFO: Initializing example.\r\n");
    LOCAL_exampleInit(PRU_NUM);
    

    /* initialize memo */

   memset(dumpTargetMemo, 0x61, 10000);
   memset((char *)sharedMem_int, 0x00, 10000);
    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    
    DDR_paramaddr = ddrMem + OFFSET_DDR - 8;
    DDR_ackaddr = ddrMem + OFFSET_DDR - 4;
    
    sharedMem_int[OFFSET_SHAREDRAM]=0; // set to zero means no command
    // Execute program
    prussdrv_exec_program (PRU_NUM, "./prucode_adc.bin");
		printf("Executing. \n");
		sleep(1);
		int cycleNumber=CYCLE;
		printf("curr=%d", sharedMem_int[OFFSET_SHAREDRAM]);
		sharedMem_int[OFFSET_SHAREDRAM]=(unsigned int)2; // set to 2 means perform capture
                sharedMem_int[OFFSET_SHAREDRAM+1]=(unsigned int)3;
	//	printf("curr=%d", sharedMem_int[OFFSET_SHAREDRAM]);
		// give some time for the PRU code to execute
	//	sleep(1);
		printf("Waiting for ack (curr=%d). \n", sharedMem_int[OFFSET_SHAREDRAM]);


		if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
		{	
		    printf("Error: Connect Failed\n");
		}
            
	        //int dumpOffset=0; 
                int mergeOffset=0, timer=0;
		do
		{     
                       // timer++;
                        if(timer>40000000){
                        printf("%d\n", cycleNumber);
                        break;
                        }
 
			if ( sharedMem_int[OFFSET_SHAREDRAM] == 1 )
			{
				// we have received the ack
                                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
				write(sockfd,(char *)&sharedMem_int[OFFSET_SHAREDRAM+2], UNITLENGTH);
                                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                               // memcpy(dumpTargetMemo+dumpOffset, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2], UNITLENGTH);
                               // merge(dumpOffset, mergeOffset);
                               // mergeOffset+=UNITLENGTH/8;

				sharedMem_int[OFFSET_SHAREDRAM]=(unsigned int)3; //signal PRU to continue                                
				dumpOffset+=UNITLENGTH;
				cycleNumber--;
                               
			//printf("Ack=1, cycleNumber=%d,block 1 dumped, time=%d\n",cycleNumber, end.tv_nsec-start.tv_nsec);
			}
                        
                        if ( sharedMem_int[OFFSET_SHAREDRAM+1] == 1 ) 
                        {
                                // we have received the ack!
                                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
				write(sockfd, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2]+UNITLENGTH, UNITLENGTH);
                                //clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                               // memcpy(dumpTargetMemo+dumpOffset, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2]+UNITLENGTH, UNITLENGTH);
                               // merge(dumpOffset, mergeOffset);
                               // mergeOffset+=UNITLENGTH/8;
                                sharedMem_int[OFFSET_SHAREDRAM+1]=(unsigned int)3; //signal PRU to continue                 
                                dumpOffset+=UNITLENGTH;
                                cycleNumber--;
                                
                        //printf("Ack=1, cycleNumber=%d,block 2 dumped, time=%d\n",cycleNumber, end.tv_nsec-start.tv_nsec);
                        } 

			if ( sharedMem_int[OFFSET_SHAREDRAM] == 4 )
                        {
                                // we have received the ack!

                               // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
				write(sockfd, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2], UNITLENGTH);
                                memcpy(dumpTargetMemo+dumpOffset, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2], UNITLENGTH);
                               // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                               merge(dumpOffset, mergeOffset);
                                mergeOffset+=UNITLENGTH/8;

                                sharedMem_int[OFFSET_SHAREDRAM]=(unsigned int)3; //signal PRU to continue
                                dumpOffset+=UNITLENGTH;
                                cycleNumber--;
                         //       printf("Ack=1, cycleNumber=%d, BLK 1 PACKET LOST\n", cycleNumber);

                        }

                        if ( sharedMem_int[OFFSET_SHAREDRAM+1] == 4 )
                        {
                                // we have received the ack!
                               // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
                                write(sockfd, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2]+UNITLENGTH, UNITLENGTH);
				memcpy(dumpTargetMemo+dumpOffset, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2]+UNITLENGTH, UNITLENGTH);
                                merge(dumpOffset, mergeOffset);
                                mergeOffset+=UNITLENGTH/8;

                               // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
                                sharedMem_int[OFFSET_SHAREDRAM+1]=(unsigned int)3; //signal PRU to continue
                                dumpOffset+=UNITLENGTH;
                                cycleNumber--;

                  //              printf("Ack=1, cycleNumber=%d, BLK 2 PACKET LOST\n", cycleNumber);

                        }



                       
		} while(cycleNumber>0);
    		
                       
                outfile=fopen("data.csv", "w");
	//	printf("error=%d",dump2FileCSV());
                dump2FileCSV2();
                // dump2FileCSV();
                 fclose(outfile);

                outtxt=open("data.mp3", O_WRONLY|O_CREAT);
                dump2FileTXT();
                close(outtxt);

		
    //prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("Done\n");
    	//prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

 		   	

		//fclose(outfile);
           //     fclose(outtxt);
    
    
    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM); 
    prussdrv_exit ();
    munmap(ddrMem, 0x0FFFFFFF);
    close(mem_fd);

    return(0);
}

/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit (  )
{
    void *DDR_regaddr1, *DDR_regaddr2, *DDR_regaddr3;	
    
    prussdrv_map_prumem(PRUSS1_SHARED_DATARAM, &sharedMem);
    sharedMem_int = (unsigned int*) sharedMem;

    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return -1;
    }	

    /* map the DDR memory */
    ddrMem = mmap(0, 0x0FFFFFFF, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, DDR_BASEADDR);
    if (ddrMem == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(mem_fd);
        return -1;
    }
    

    return(0);
}
