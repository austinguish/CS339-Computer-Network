#include <stdio.h>  // standard input and output library
#include <stdlib.h> // this includes functions regarding memory allocation
#include <string.h> // contains string functions
#include <errno.h> //It defines macros for reporting and retrieving error conditions through error codes
#include <time.h> //contains various functions for manipulating date and time
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate
#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses
#include <pthread.h>
const int BUFFER_SIZE = 1024;
void sendfile(void *socket_fd) {
    clock_t start;
    int fd = *((int *) socket_fd);
    struct sockaddr_in sa;
    int len = sizeof(sa);
    getpeername(fd, (struct sockaddr *)&sa, reinterpret_cast<socklen_t *>(&len));
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    if(recv(fd,buffer,BUFFER_SIZE,0)<0) {
        perror("Server Receive Data Failed!");
    }
    char file_name[FILENAME_MAX+1];
    bzero(file_name,FILENAME_MAX+1);
    strncpy(file_name,buffer,strlen(buffer)>FILENAME_MAX?FILENAME_MAX:strlen(buffer));
    printf("%s\n",file_name);
    FILE *fp = fopen(file_name,"r");
    if(!fp) {
        printf("File:%s Not Found!",file_name);
    }
    else {
        bzero(buffer,BUFFER_SIZE);
        int length = 0;
        while((length=fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0) {
            if(send(fd,buffer,length,0)<0){
                printf("Send File:%s.\n",file_name);
                break;
            }
            bzero(buffer,BUFFER_SIZE);
        }
        fclose(fp);
        printf("to host %s cost %ld ms",inet_ntoa(sa.sin_addr),(clock()-start)/1000); 
        printf("File:%s Transfer Successful!\n",file_name);
    }
    close(fd);

}
int main()
{
    time_t clock;
	char dataSending[1025]; // Actually this is called packet in Network Communication, which contain data and send through.
	int clintListn = 0, clintConnt = 0;
	struct sockaddr_in ipOfServer;
	clintListn = socket(AF_INET, SOCK_STREAM, 0); // creating socket
	memset(&ipOfServer, '0', sizeof(ipOfServer));
	memset(dataSending, '0', sizeof(dataSending));
	ipOfServer.sin_family = AF_INET;
	ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY);
	ipOfServer.sin_port = htons(2017); 		// this is the port number of running server
	bind(clintListn, (struct sockaddr*)&ipOfServer , sizeof(ipOfServer));
	listen(clintListn , 20);
 
	while(1)
	{
	    pthread_t thread_id;
		printf("Hi,Iam running server.Some Client hit me\n"); // whenever a request from client came. It will be processed here.
		clintConnt = accept(clintListn, (struct sockaddr*)NULL, NULL);
		if(clintConnt==-1) {
		    fprintf(stderr,"Accept error!\n");
            continue;
		}
        struct sockaddr_in sa;
        int len = sizeof(sa);

        if(!getpeername(clintConnt, (struct sockaddr *)&sa, reinterpret_cast<socklen_t *>(&len)))
        {
            printf("New connection from %s:%d !\n",inet_ntoa(sa.sin_addr),ntohs(sa.sin_port));
        }

		if(pthread_create(&thread_id, NULL, reinterpret_cast<void *(*)(void *)>(sendfile), (void *)(&clintConnt))) {
		    fprintf(stderr,"pthread create error");
		    break;
		}
     }
	return 0;
}
