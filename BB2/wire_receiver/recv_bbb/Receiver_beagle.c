//UNITLENGTH changed for testing purpose

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
#define PORT_LAPTOP 4997
#define OFFSET_SHAREDRAM 0
#define PRUSS1_SHARED_DATARAM    4
#define CYCLE 400
#define UNITLENGTH 4992
//#define UNITLENGTH 612

//original  data 4800, bit 4801~4832 current cone, bit 4833-4864 prevCone, bit 4865-4896 ip address, bit 4897-4928 packet ID


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

static unsigned char dumpTargetMemo[150000000];
static char playFrom[1500000];
FILE* outfile, * outtxt;

/******************************************************************************
* Global Function Definitions                                                 *
******************************************************************************/

int dumpOffset = 0;

//void dump2FileMP3(void)
//{
//   printf("dumping data from memory to TXTfile...\n");
//   if(write(outtxt, dumpTargetMemo, UNITLENGTH*CYCLE)==UNITLENGTH*CYCLE)
//   {
//     printf("dump to txt success!\n");
//    }

//}

int bit2byte(unsigned char *byte, unsigned char *bit, int size)

{
                 int i,j=0, k = 0, N_Bytes=size/8;
                 unsigned char mask = 0x01;
                 memset(byte, 0x00, N_Bytes);
                 for(i=0 ; i<size ; i++)
                 {
                         if(bit[i])
                         byte[k] = byte[k] | mask;
                         mask = mask<<1;
                         j++;
			 if((j%8)==0)
			 {
                       		mask = 0x01;
                                k++;
			 }
		 }
		if(k != N_Bytes)
		return -1;
		else
		return k;
}

void dump2FileTXT(void)
{
	int i=0, j;
	while (i<(UNITLENGTH/2-4)*CYCLE)
	{
		for(j=0; j<8; j++)
		{
			if(dumpTargetMemo[i] == 0x40)
				fprintf(outtxt, "1");
			else
			{
				if(dumpTargetMemo[i] == 0x00)
				fprintf(outtxt, "0");
				else
					fprintf(outtxt, "8");
			}
			i++;
		}
		fprintf(outtxt,"\n");
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
   for (x=0; x<(UNITLENGTH/2)*CYCLE; x++)
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


int mc_decoder(unsigned char * coded, unsigned char * decoded, int Ninput)
{
	int i, j=0;
        //unsigned char temp[2];

	for(i=0 ; i<Ninput ;)
	{
		//memcpy(temp,coded+i,2);
		if((coded[i] == 0x00) && (coded[i+1]==0x40))
		{
			decoded[j] = 0x40;
			i=i+2;
		}
		else if((coded[i] == 0x40) && (coded[i+1]==0x00))
		{
			decoded[j] = 0x00;
			i=i+2;
		}
		else if((coded[i] == 0x00) && (coded[i+1]==0x00))
		{
                        decoded[j] = 0x40;
			i=i+1;
		}
		else if((coded[i] == 0x40) && (coded[i+1]==0x40))
		{
			decoded[j] = 0x00;
			i=i+1;
		}

		j++;
       	}

	 return j;
}

int main (void)
{
    int writeCount, writeTotal=0;
    unsigned int ret;
    int i;
    void *DDR_paramaddr;
    void *DDR_ackaddr;
    int fin;
    char fname_new[255];
    struct timespec  start, end;
    double duration;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    char disconnectSignal[UNITLENGTH];
    unsigned char MC_decoded[UNITLENGTH/2];
    long timeDiff;

		unsigned char packet_bit[624*8];
		unsigned char packet_byte[624];

		int printCounter, bitCounter;;








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

   memset(dumpTargetMemo, 0x61, 150000000);
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
		memset(sharedMem_int, 0x61, 11000);
		sharedMem_int[OFFSET_SHAREDRAM]=(unsigned int)2; // set to 2 means perform capture
                sharedMem_int[OFFSET_SHAREDRAM+1]=(unsigned int)3;
	//	printf("curr=%d", sharedMem_int[OFFSET_SHAREDRAM]);
		// give some time for the PRU code to execute
	//	sleep(1);
		printf("Waiting for ack (curr=%d). \n", sharedMem_int[OFFSET_SHAREDRAM]);


    int packetNumber=0;
		int packetCount=0;
		int counterJ=0;
		do
		{
		        counterJ= 0;
			//if ( sharedMem_int[OFFSET_SHAREDRAM] == 1 )
			while(1)
			{
				if ( sharedMem_int[OFFSET_SHAREDRAM] == 1 )
					break;

			}
			printf("0 received!\n");
			{
				for (bitCounter = 0; bitCounter<32; bitCounter++)
				{
					printf("0x%x ", *((unsigned char *)&(sharedMem_int[OFFSET_SHAREDRAM+2])+bitCounter));
				}
				printf("\n");

/*				for(;counterJ<64;counterJ++)
				{
					printf("0x%x\n", *((unsigned char *)&sharedMem_int[OFFSET_SHAREDRAM+2]+counterJ));
				}
				counterJ=0; */

                        //        printf("%d received\n", cycleNumber);
			//	mc_decoder((unsigned char *)&(sharedMem_int[OFFSET_SHAREDRAM+2]), MC_decoded, UNITLENGTH);
/*				printf("decoded!\n");
				for(;counterJ<32;counterJ++)
				{
					printf("0x%x\n", MC_decoded[counterJ]);
				}*/
			//	printf("start\n");
				clock_gettime(CLOCK_MONOTONIC, &start);
				bit2byte(packet_byte, (unsigned char *)&(sharedMem_int[OFFSET_SHAREDRAM+2]), 624*8);
			
				for (printCounter = 0; printCounter<16; printCounter++)
				{
					printf("0x%x ", packet_byte[printCounter]);
				}
				printf("\n");
			//	clock_gettime(MONOTONIC, &end);

			//	printf("%d write to laptop!\n", writeTotal);

//				bit2byte((unsigned char *)(&packetNumber), MC_decoded, 32);
                              //  packetNumber = *(int *)MC_decoded;
			//	printf("packet Number %d, batch 0\n", packetNumber);

		//		for(; counterJ<64; counterJ++)
		//		{
		//			printf("0x%x\n", MC_decoded[counterJ]);
		//		}
	//*			memcpy(dumpTargetMemo+packetNumber*(UNITLENGTH/2-32), MC_decoded+32, UNITLENGTH/2-32);
				sharedMem_int[OFFSET_SHAREDRAM]=(unsigned int)3; //signal PRU to continue
			//	dumpOffset+=UNITLENGTH;
		//		packetCount++;
		//		printf("packetCount = %d\n", packetCount);
//				if (packetNumber == 399)
//					break;
             clock_gettime(CLOCK_MONOTONIC, &end);
			       timeDiff = (end.tv_sec-start.tv_sec)*1000000000+end.tv_nsec-start.tv_nsec;
			      // if (timeDiff > 1000000)
				      // printf("%ld\n", timeDiff);

			//printf("Ack=1, cycleNumber=%d,block 1 dumped, time=%d\n",cycleNumber, end.tv_nsec-start.tv_nsec);
			}

                        counterJ=0;

			//if ( sharedMem_int[OFFSET_SHAREDRAM+1] == 1 )
			while(1)
			{
				if ( sharedMem_int[OFFSET_SHAREDRAM+1] == 1 )
				       break;
			}
			printf("1 received!\n");
				for (bitCounter = 0; bitCounter<32; bitCounter++)
				{
					printf("0x%x ", *((unsigned char *)&(sharedMem_int[OFFSET_SHAREDRAM+2])+624*8+bitCounter));
				}
				printf("\n");


				bit2byte(packet_byte, (unsigned char *)&(sharedMem_int[OFFSET_SHAREDRAM+2])+624*8, 624*8);
				/*for (printCounter = 0; printCounter<16; printCounter++)
				{
					printf("0x%x ", packet_byte[printCounter]);
				}
				printf("\n");*/
			//	printf("%d write to laptop!****\n", writeTotal);


			//	packetNumber = *(int *)MC_decoded;
//				bit2byte((unsigned char *)(&packetNumber), MC_decoded, 32);
			//	printf("packet Number %d received!, batch 1\n", packetNumber);
	//*			memcpy(dumpTargetMemo+packetNumber*(UNITLENGTH/2-32), MC_decoded+32, UNITLENGTH/2-32);
			//	memcpy(dumpTargetMemo, (char *)&sharedMem_int[OFFSET_SHAREDRAM+2]+UNITLENGTH, UNITLENGTH);
                          //      printf("%d\n",writeCount);
				sharedMem_int[OFFSET_SHAREDRAM+1]=(unsigned int)3; //signal PRU to continue
			//	dumpOffset+=UNITLENGTH;
                  //              packetCount++;
		//		printf("packetCount = %d\n", packetCount);
//				if (packetNumber =:x
//				= 399)
//					break;


		} while(1);

		printf("%d packets received\n", packetCount);

                outfile=fopen("data.csv", "w");
	//	printf("error=%d",dump2FileCSV());
                dump2FileCSV2();
                // dump2FileCSV();
                 fclose(outfile);

                outtxt=fopen("data.txt", "w");
                dump2FileTXT();
                fclose(outtxt);


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
