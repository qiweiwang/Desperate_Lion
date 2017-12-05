
/*
 * mytest.c
 */

/******************************************************************************
* Include Files                                                               *
******************************************************************************/

// Standard header files
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
//#include <i2cfunc.h>

// Driver header file
#include "prussdrv.h"
#include <pruss_intc_mapping.h>
/******************************************************************************
* Explicit External Declarations                                              *
******************************************************************************/

/******************************************************************************
* Local Macro Declarations                                                    *
******************************************************************************/

#define PRU_NUM 	 0
#define ADDEND1	 	 0x98765400u
#define ADDEND2		 0x12345678u
#define ADDEND3		 0x10210210u
#define LOADLENGTH       4800
#define DDR_BASEADDR     0x80000000
#define OFFSET_DDR	 0x00001008
#define OFFSET_SHAREDRAM 0		//equivalent with 0x000020
#define READSIZE	 24000*4800
#define PRUSS1_SHARED_DATARAM    4
#define DATASIZE 2400
#define HEADERSIZE 96
#define SOCKET_BEACON 5005
#define DELAY 1000000

/******************************************************************************
* Local Typedef Declarations                                                  *
******************************************************************************/


/******************************************************************************
* Local Function Declarations                                                 *
******************************************************************************/

static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/******************************************************************************
* Local Variable Definitions                                                  *
******************************************************************************/


/******************************************************************************
* Intertupt Service Routines                                                  *
******************************************************************************/


/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static int mem_fd;
static void *ddrMem, *sharedMem;
static char pktBuffer[HEADERSIZE+DATASIZE];
static unsigned int *sharedMem_int;
static int readFile;
/******************************************************************************
* Global Function Definitions                                                 *
******************************************************************************/


int NEWbyte2bit(unsigned char *byte, unsigned char *bit, int size)
{
	int i,j, N_Bits=0;

	for(i=0 ; i<size ; i++)
	{
		for(j=0 ; j<8 ; j++)
		{

				bit[N_Bits] = (byte[i]&0x01)<<6;
			byte[i] = byte[i]>>1;
			N_Bits++;
		}
	}

	return N_Bits;
}





int main (void)
{
    struct timespec start, end;
    unsigned int ret;
    int i;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    int readCount, readTotal;
    readTotal=0;
    int packetCount;
    packetCount=0;
    char rawData[(HEADERSIZE+DATASIZE)/8];
    unsigned char recv_buffer[(HEADERSIZE+DATASIZE)*2];
    long timeDiff;
    struct timespec delayTime;
    unsigned char datatosend[100];



    // make a single packet for transmission
		unsigned char packet_byte[624];
		unsigned char packet_bit[624*8];

                packet_byte[0]= 0x71;
		packet_byte[1]= 0x51;
		packet_byte[2]= 0x34;
		packet_byte[3]= 0x2a;

		packet_byte[4]= 0xff;
		packet_byte[5]= 0xa4;
		packet_byte[6]= 0x2b;
		packet_byte[7]= 0xaa;

		packet_byte[8]= 0x7d;
		packet_byte[9]= 0x5c;
		packet_byte[10]= 0xaa;
		packet_byte[11]= 0xb9;

		packet_byte[12]= 0xd2;
		packet_byte[13]= 0x3c;
		packet_byte[14]= 0xcf;
		packet_byte[15]= 0xa6;

		memset(packet_byte+16, 0xab, 624-16);
		NEWbyte2bit(packet_byte, packet_bit, 624);









    printf("\nINFO: Starting %s example.\r\n", "PRU_memAccess_DDR_PRUsharedRAM");
    /* Initialize the PRU */
    prussdrv_init ();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_1);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);



    /* Initialize example */
    printf("\tINFO: Initializing example.\r\n");
    LOCAL_exampleInit(PRU_NUM);

    /* Execute example on PRU */
    printf("\tINFO: Executing example.\r\n");
    int loadOffset=0;
    /* Set header */
    sharedMem_int[2]=0x40004000;
    sharedMem_int[3]=0x00004000;
    sharedMem_int[4]=0x40400040;
    sharedMem_int[5]=0x40000040;
    sharedMem_int[6]=0x00004000;
    sharedMem_int[7]=0x00404040;
    sharedMem_int[8]=0x00404000;
    sharedMem_int[9]=0x40004000;
    memcpy(&(sharedMem_int[10+(HEADERSIZE+DATASIZE)/2]),&(sharedMem_int[2]),32);
   //sharedMem_int[3+(HEADERSIZE+DATASIZE)/4]=0x654eb954;
    int counter=0,totalTime=0,delayCounter;
    prussdrv_exec_program (PRU_NUM, "./prucode.bin");



    int counterR = 0;
    int counterS = -2;
    int delay = 0;
    int counterJ=0;
    int bitCounter, byteCounter;



    delayTime.tv_sec = 0;
    delayTime.tv_nsec = 100000;
    sharedMem_int[0] = 1;
    sharedMem_int[1] = 1;
    printf("start!!!!!!!!\n");
   //clock_gettime(CLOCK_MONOTONIC, &start);
    do{


			if(sharedMem_int[0]==1)
			{
				// printf("packet %d sent!, batch 0 empty\n", counterS);
                                 counterS++;
                                // nanosleep(&delayTime, NULL);
                                 sleep(1);
							memcpy((unsigned char *)(sharedMem_int+10), packet_bit, 624*8);
                                
				// memset((unsigned char *)(sharedMem_int+10), 0x40, 624*8);
				 for(bitCounter = 0; bitCounter<32; bitCounter++)
				 {
					printf("0x%x ", *((unsigned char *)(sharedMem_int+10)+bitCounter));
				 }
				 printf("\n");
	      			 sharedMem_int[0]=2;
              			 printf("packet %d loaded!\n", counterR);
              			 counterR++;

              			 counterJ=0;
        }


			if(sharedMem_int[1]==1)
			{
			  //      printf("packet %d sent! batch 1 empty\n", counterS);
				counterS++;





                                // clock_gettime(CLOCK_MONOTONIC, &start);
				// nanosleep(&delayTime, NULL);
				sleep(1);
				
				memcpy((unsigned char *)(sharedMem_int+18)+624*8, packet_bit, 624*8);

              		//	 MC_encoder(recv_buffer, (unsigned char *)(sharedMem_int+18)+(HEADERSIZE+DATASIZE)*2, sizeof(recv_buffer));
   /*          for(; counterJ<64; counterJ++)
		{
			printf("0x%x\n", *((unsigned char *)(sharedMem_int+18)+HEADERSIZE+DATASIZE+counterJ));
			if(counterJ%2 ==1)
			printf("\n");
		}
     */
              			 sharedMem_int[1]=2;
              		//	 printf("packet %d loaded!\n", counterR);
              			 counterR++;
              			 counterJ=0;
				// clock_gettime(CLOCK_MONOTONIC, &end);
		                // timeDiff = (end.tv_sec-start.tv_sec)*1000000000+end.tv_nsec-start.tv_nsec;
				// printf("%ld\n", timeDiff);
			}
       }while(1);
       printf("end!!!!!!!!!\n");

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");

    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    /* prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT);

    /* Check if example passed */
    if ( LOCAL_examplePassed(PRU_NUM) )
    {
        printf("Example executed succesfully.\r\n");
    }
    else
    {
        printf("Example failed.\r\n");
    }

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

    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR|O_SYNC);
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

    /* Allocate Shared PRU memory. */
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);
    sharedMem_int = (unsigned int*) sharedMem;


    /* Store Addends in DDR memory location */
    DDR_regaddr1 = ddrMem + OFFSET_DDR;
    DDR_regaddr2 = ddrMem + OFFSET_DDR + 0x00000004;
    DDR_regaddr3 = ddrMem + OFFSET_DDR + 0x00000008;

    *(unsigned long*) DDR_regaddr1 = ADDEND1;
    *(unsigned long*) DDR_regaddr2 = ADDEND2;
    *(unsigned long*) DDR_regaddr3 = ADDEND3;

    return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
    unsigned int result_0, result_1, result_2;

     /* Allocate Shared PRU memory. */
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);
    printf("%x\n", sharedMem);
    sharedMem_int = (unsigned int*) sharedMem;

    result_0 = sharedMem_int[OFFSET_SHAREDRAM];
    result_1 = sharedMem_int[OFFSET_SHAREDRAM + 1];
    result_2 = sharedMem_int[OFFSET_SHAREDRAM + 2];

    return ((result_0 == ADDEND1) & (result_1 ==  ADDEND2) & (result_2 ==  ADDEND3)) ;

}
