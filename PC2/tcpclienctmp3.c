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
    int sockfd = 0, readCount, fileID;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 


    fileID = open("abc.mp3", O_RDWR|O_CREAT);
    printf("fileID is %d\n", fileID);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
    memset(&recvBuff, '0', sizeof(recvBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(2077); 

    if(inet_pton(AF_INET, "192.168.0.211", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    } 

    printf("connecting...\n");

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       printf("%s\n", strerror(errno));
       return;
    }
    else
    {
	printf("connect succuss!\n");
    }

    while(1)
    {
        readCount=read(sockfd, recvBuff, 1024);
        if (readCount==-1)
            printf("ERROR readCount = -1\n");
        else
        {
            printf("readCount=%d\n", readCount);
            write(fileID, recvBuff, readCount);
/*
            int i=0;
            while (readCount>0)
            {
                //printf("%X ", recvBuff[i]);
                i++;
                readCount--;
            }
*/
        }
        //printf("\n");
 
    }

    close(sockfd);
    close(fileID);

}
