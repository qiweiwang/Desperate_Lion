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


int main()
{
    int sockfd = 0, n = 0, fd=0;
    char recvBuff[2500];
    struct sockaddr_in serv_addr; 
    int readCount, readTotal = 0;
    int PACKETSIZE = 2500;

    memset(&serv_addr, '0', sizeof(serv_addr)); 


    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5999); 

    if(inet_pton(AF_INET, "192.168.7.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return;
    } 

    while(1)
    {
    	//if(ack_flag == 1)
    	   while(readTotal < PACKETSIZE)
    		{
    			readCount=read(sockfd, recvBuff+readTotal, PACKETSIZE-readTotal);
                        if(readCount >= 0)
    			    readTotal += readCount;
                        else
                            printf("read error -1\n");
    		}
    		readTotal = 0;
    		
    	
    } 





    return 0;
}
