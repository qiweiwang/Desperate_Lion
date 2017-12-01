#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>

#include "prussdrv.h"
#include <pruss_intc_mapping.h>	





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



/*****************************************************************************
 PRU Global Variable Definitions                                               
*************************************************************************/
static int mem_fd;
static void *ddrMem, *sharedMem;
static char pktBuffer[HEADERSIZE+DATASIZE];
static unsigned int *sharedMem_int;
static int readFile;

//**********************************************************************





//*********************PRU function declaration*************
static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );
//*****************************************************************



/*****************************************************************************
* PRU Local Function Definitions                                                 *
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


//************************************************************************



/**********************************************************
socket programming function definitions
**************************************************/


int getLength(unsigned char *lengthC);
void getChecksum(unsigned char *buf, unsigned char *checksum, int count);

int getLength(unsigned char *lengthC)
{
    int length = 0;
    unsigned char LenC[4];
    
    LenC[0] = lengthC[1];
    LenC[1] = lengthC[0];
    LenC[2] = 0x00;
    LenC[3] = 0x00;
    length = *(int *)LenC;

    return length;
}

void getChecksum(unsigned char *buf, unsigned char *checksum, int count)
{
    unsigned char sum[4], temp[4];
    int i;
    //unsigned int sum = 0, temp=0, i =0;

    *(unsigned int*)sum = 0;
    *(unsigned int*)temp = 0;

    for(i=0 ; i<count ;i++)
    {
        temp[2] = 0x00;
        temp[3] = 0x00;
        temp[1] = buf[i*2];
        temp[0] = buf[i*2+1];

        //printf("%x  %x  %x  %x\n", temp[3], temp[2], temp[1], temp[0]);

        *(unsigned int*)sum = *(unsigned int*)sum + *(unsigned int*)temp;
        *(unsigned int*)sum = (*(unsigned int*)(sum)&0x0000ffff) + (*(unsigned int*)(sum)>>16);
    }

    
    checksum[0] = ~sum[1];
    checksum[1] = ~sum[0];

    return;

}

void getOpChecksum(unsigned char * buf, unsigned char * checksum, int count)
{
	return;
}



int main()
{

	//socket variable declaration
    struct sockaddr peer, from;
    struct sockaddr myaddr, myaddr2;
    socklen_t peerlen = 0, fromlen=0;
    int client, s, rc, on=1, opt = 1, r=0;
    unsigned char msg[2048];
    unsigned char temp[2048];
    unsigned char lengthC[2], srcMAC[6], dstMAC[6], dstIP[4];
    int length=0;
    int tlen;
    
    
    //PRU variable declaration
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
    int counterR = 0;
    int counterS = -2;
    int delay = 0;
    int counterJ=0;
    delayTime.tv_sec = 0;
    delayTime.tv_nsec = 100000;
    int counter=0,totalTime=0,delayCounter;
    
    
    
    //*******************PRU initialization********************************//
    //**********************************************************************
    
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

    
    prussdrv_exec_program (PRU_NUM, "./prucode.bin");
    sharedMem_int[0] = 1;
    sharedMem_int[1] = 1;
    printf(" PRU stand by......\n");
   // ******************************************************************** 
    
    
  

    
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sa_family = AF_INET;
    strcpy(myaddr.sa_data, "eth0");



    s = socket(AF_INET, SOCK_PACKET, htons(0x0003));
    printf("s=%d\n", s);
    if(s<0)
    {
	printf("socket call failed\n");
        printf("%d : %s\n", errno, strerror(errno));
        exit(1);
    }


    
    rc = bind(s, &myaddr, sizeof(struct sockaddr));
    printf("bind rc = %d", rc);
    if(rc<0)
    {
        printf("bind call failed\n");
        exit(1);
    }
 

    memset(msg, 0x00, 2048);
    
    //PCII eth0
    dstMAC[0] = 0x00;
    dstMAC[1] = 0x23;
    dstMAC[2] = 0x18;
    dstMAC[3] = 0xa5;
    dstMAC[4] = 0x32;
    dstMAC[5] = 0x97;

    //PCI eth8
    srcMAC[0] = 0x84;
    srcMAC[1] = 0x16;
    srcMAC[2] = 0xf9;
    srcMAC[3] = 0x04;
    srcMAC[4] = 0x49;
    srcMAC[5] = 0x08;

    //PCII eth0 IP 192.168.2.100
    dstIP[0] = 0xc0;
    dstIP[1] = 0xa8;
    dstIP[2] = 0x02;
    dstIP[3] = 0x64;

    while(1)
    {
        memset(msg, 0x00, 2048);
	printf("frsafjksfdsfsda\n");
        r = recvfrom(s, msg, 2048, 0, &peer, &peerlen);
        memcpy(lengthC, msg+16, 2);
        length = getLength(lengthC);

        
        


        
        if (r==-1)
            printf("ERROR r = -1\n");
        else
        {
            printf("r=%d\n", r);
            i=0;
            while (r>0)
            {
                printf("%X ", msg[i]);
                i++;
                r--;
            }
        }
        printf("\n");
        

        memcpy(msg, dstMAC, 6);
        memcpy(msg+6, srcMAC, 6);
        memcpy(msg+30, dstIP, 4);

        //compute  IP checksum
        msg[24] = 0x00;
        msg[25] = 0x00;
        getChecksum(msg+14, msg+24, 10);

        //compute TCP checksum
        memset(temp, 0x00, 2048);
        msg[50] = 0x00;
        msg[51] = 0x00;

        memcpy(temp, msg+26, 4);
        memcpy(temp+4, msg+30, 4);
        memcpy(temp+9, msg+23, 1);
        tlen = length-20;
        temp[10] = ((unsigned char*)(&tlen))[1];
        temp[11] = ((unsigned char*)(&tlen))[0];         

        memcpy(temp+12, msg+34, tlen);
        if(tlen&1)
            getChecksum(temp, msg+50, (tlen+12+1)/2);
        else
            getChecksum(temp, msg+50, (tlen+12)/2);
 

    //send to optical here
    /*    do{       		  

                        
			if(sharedMem_int[0]==1)
			{      
                    counterS++;
                    usleep(100);
	       		//	makeOpticalPacket();                        
	      			sharedMem_int[0]=2; 
              		counterR++;
                    counterJ=0;
            }
 
       }while(1);*/


    
    
    
    
    

    }
    close(s);

}
