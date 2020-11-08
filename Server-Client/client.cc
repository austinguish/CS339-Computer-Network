#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
const int BUFFER_SIZE=1024;
int main()
{
    clock_t  start = clock();
    int CreateSocket = 0,n = 0;
    char dataReceived[1024];
    struct sockaddr_in ipOfServer;
 
    memset(dataReceived, '0' ,sizeof(dataReceived));
 
    if((CreateSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        return 1;
    }
 
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(2017);
    ipOfServer.sin_addr.s_addr = inet_addr("10.0.0.7");
 
    if(connect(CreateSocket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer))<0)
    {
        printf("Connection failed due to port and ip problems\n");
        return 1;
    }
    char file_name[FILENAME_MAX+1]={"file"};
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE?strlen(file_name):BUFFER_SIZE);
    if (send(CreateSocket,buffer,BUFFER_SIZE,0)<0) {
        perror("Send File Name Failed:");
        exit(1);
    }
    FILE *fp = fopen(file_name,"a+x");
    if(!fp) {
        printf("Can not open file:\t %s",file_name);
        exit(1);
    }
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    while((length = recv(CreateSocket,buffer,BUFFER_SIZE,0))>0) {
        if(fwrite(buffer,sizeof(char),length,fp)<length) {
            printf("Failed\n");
            break;
        }
        bzero(buffer,BUFFER_SIZE);
    }
    printf("Receive done\n");
    fclose(fp);
    printf("TIME COST %ld ms",(clock()-start)/1000);
    close(CreateSocket);
    return 0;
}
