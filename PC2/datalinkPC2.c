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


int main()
{
    //struct sockaddr_in local;
    struct sockaddr peer, from;
    struct sockaddr myaddr, myaddr2;
    socklen_t peerlen = 0, fromlen=0;
    int client, s, s2, rc, on=1, opt = 1, r=0;
    struct timespec start, end;
    unsigned char msg[2048];
    unsigned char temp[2048];
    unsigned char lengthC[2], srcMAC[6], dstMAC[6], dstIP[4], srcIP[4], B2MAC[7], B2IP[5], tempMAC[7], tempIP[5];
    int length=0;
  

    //memset(&local, 0, sizeof(local));
    memset(&myaddr, 0, sizeof(myaddr));
    memset(&myaddr2, 0, sizeof(myaddr2));
    myaddr.sa_family = AF_INET;
    myaddr2.sa_family = AF_INET;
    strcpy(myaddr.sa_data, "eth0");
    strcpy(myaddr2.sa_data, "wlan0");

    memset(&from, 0, sizeof(from));
    from.sa_family = AF_INET;
    strcpy(from.sa_data, "wlan0");
    fromlen = sizeof(from);


    s = socket(AF_INET, SOCK_PACKET, htons(0x0003));
    printf("s=%d\n", s);
    if(s<0)
    {
	printf("socket call failed\n");
        printf("%d : %s\n", errno, strerror(errno));
        exit(1);
    }

    s2 = socket(AF_INET, SOCK_PACKET, htons(0x0003));
    printf("s=%d\n", s2);
    if(s2<0)
    {
        printf("socket 2 call failed\n");
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

    rc = bind(s2, &myaddr2, sizeof(struct sockaddr));
    printf("bind2 rc = %d", rc);
    if(rc<0)
    {
        printf("bind2 call failed\n");
        exit(1);
    }
    

    memset(msg, 0x00, 2048);
    
    //PCII eth0
    B2MAC[0] = 0x00;
    B2MAC[1] = 0x23;
    B2MAC[2] = 0x18;
    B2MAC[3] = 0xa5;
    B2MAC[4] = 0x32;
    B2MAC[5] = 0x97;
    B2MAC[6] = '\0';

    //PCII wlan0
    srcMAC[0] = 0x58;
    srcMAC[1] = 0x94;
    srcMAC[2] = 0x6b;
    srcMAC[3] = 0x35;
    srcMAC[4] = 0x9b;
    srcMAC[5] = 0x9c;

    //router lan
    dstMAC[0] = 0x14;
    dstMAC[1] = 0xcc;
    dstMAC[2] = 0x20;
    dstMAC[3] = 0xe2;
    dstMAC[4] = 0x34;
    dstMAC[5] = 0x28;

    //PCII eth0 IP 192.168.2.100
    B2IP[0] = 0xc0;
    B2IP[1] = 0xa8;
    B2IP[2] = 0x02;
    B2IP[3] = 0x64;
    B2IP[4] = '\0';

    //PCII wlan0 IP 192.168.0.100
    srcIP[0] = 0xc0;
    srcIP[1] = 0xa8;
    srcIP[2] = 0x00;
    srcIP[3] = 0x64;

    //server wlan0 IP 192.168.0.211
    dstIP[0] = 0xc0;
    dstIP[1] = 0xa8;
    dstIP[2] = 0x00;
    dstIP[3] = 0xd3;

    while(1)
    {
        memset(msg, 0x00, 2048);
        r = recvfrom(s, msg, 2048, 0, &peer, &peerlen);
        //r = recvfrom(s, msg, 18, 0, &peer, &peerlen);
        memcpy(lengthC, msg+16, 2);
        length = getLength(lengthC);

        //printf("read r = %d", r);


        //r = recvfrom(s, msg+18, length-4, 0, &peer, &peerlen);
        
        
        if (r==-1)
            printf("ERROR r = -1\n");
        else
        {
            printf("r=%d\n", r);
            int i=0;
            while (r>0)
            {
                printf("%X ", msg[i]);
                i++;
                r--;
            }
        }
        printf("\n");

        memcpy(tempMAC, msg+6 ,6);
        tempMAC[6] = '\0';
        memcpy(tempIP, msg+26, 4);
        tempIP[4] = '\0';

        if ((strcmp(tempMAC, B2MAC)!=0) || (strcmp(tempIP, B2IP)!=0))
        {
            printf("PACKET FROM PC1\n"); 
            continue;
        }
        else
        {
            printf("#######################PACKET FROM PCII\n");
        }

        memcpy(msg, dstMAC, 6);
        memcpy(msg+6, srcMAC, 6);
        memcpy(msg+26, srcIP, 4);
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
        int tlen;
        tlen = length-20;
        temp[10] = ((unsigned char*)(&tlen))[1];
        temp[11] = ((unsigned char*)(&tlen))[0];         

        memcpy(temp+12, msg+34, tlen);
        if(tlen&1)
            getChecksum(temp, msg+50, (tlen+12+1)/2);
        else
            getChecksum(temp, msg+50, (tlen+12)/2);

        int i;
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); 
        for(i = 0 ; i<length+14 ; i++)
            printf("%x ", msg[i]);
        printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); 
        r = sendto(s2, msg, length+14, 0, &from, fromlen);
        //if(strcmp(peer.sa_data, "eth0")!=0)
            //printf("not from eth0");    
 
        /*
        if (r==-1)
            printf("ERROR r = -1\n");
        else
        {
            printf("r=%d\n", r);
            int i=0;
            while (r>0)
            {
                printf("%X ", msg[i]);
                i++;
                r--;
            }
        }
        printf("\n");
        */

    }
    close(s);
    close(s2);

}
