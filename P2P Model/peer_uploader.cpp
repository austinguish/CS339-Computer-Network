//
// Created by austinguish on 2020/10/12.
//
#include <iostream>
#include <string.h>
#include <vector>
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
#include <sys/file.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <thread>
using namespace std;
const string TORRENT_NAME = "seed.torrent";
const string TRACKER_IP = "10.0.0.7";
const string TRACKER_PORT = "2017";
const int BUFFER_SIZE=1024;
inline bool is_exist (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
void sendfile(void *socket_fd) {
    int fd = *((int *) socket_fd);
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
        long sendsize = 0;
        while((length=fread(buffer,sizeof(char),BUFFER_SIZE,fp))>0) {
            sendsize+=BUFFER_SIZE;
            if(send(fd,buffer,length,0)<0){
                printf("Send File:%s.\n",file_name);
                break;
            }
            bzero(buffer,BUFFER_SIZE);
        }
        fclose(fp);
        cout<<"send"<<sendsize<<endl;
        printf("File:%s Transfer Successful!\n",file_name);
    }
    close(fd);

}
void sub_downloader(const string fn, string dest_ip, string dest_port){
    cout<<dest_ip<<endl;
    cout<<dest_port<<endl;
    int CreateSocket = 0;
    char dataReceived[1024];
    struct sockaddr_in ipOfServer;
    stringstream strValue;
    strValue << dest_port;
    unsigned int portValue;
    strValue >> portValue;
    memset(dataReceived, '0' ,sizeof(dataReceived));

    if((CreateSocket = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("Socket not created \n");
        exit(0);
    }
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_port = htons(portValue);
    ipOfServer.sin_addr.s_addr = inet_addr(dest_ip.c_str());

    if(connect(CreateSocket, (struct sockaddr *)&ipOfServer, sizeof(ipOfServer))<0)
    {
        printf("Connection failed due to port and ip problems\n");
    }
    const char *file_name = fn.c_str();
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    strncpy(buffer,file_name,strlen(file_name)>BUFFER_SIZE?strlen(file_name):BUFFER_SIZE);
    if (send(CreateSocket,buffer,BUFFER_SIZE,0)<0) {
        perror("Send File Name Failed:");
        exit(1);
    }
    ofstream outFile;
    outFile.open(file_name); // 打开模式可省略
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    while((length = recv(CreateSocket,buffer,BUFFER_SIZE,0))>0) {
        outFile<<buffer;
        bzero(buffer,BUFFER_SIZE);
    }
    printf("Receive done\n");
    outFile.close();
    close(CreateSocket);
}
void downloader(){
    int download_cnt = 0;
    ifstream inFile("seed.torrent", ios::in);
    string lineStr;
    vector<vector<string>> strArray;
    while (getline(inFile, lineStr))
    {
        stringstream ss(lineStr);
        string str;
        vector<string> lineArray;
        // 按照逗号分隔
        while (getline(ss, str, ','))
            lineArray.push_back(str);
        strArray.push_back(lineArray);
    }
    thread downloadThreads[5];
    for (auto i : strArray)
    {
        if(is_exist(i[0])) continue;else {
            downloadThreads[download_cnt] = thread(sub_downloader,i[0],i[1],i[2]);
            downloadThreads[download_cnt].join();
        }
        ++download_cnt;

    }

}
void share(int port_num){
    time_t clock;
    char dataSending[1025]; // Actually this is called packet in Network Communication, which contain data and send through.
    int clintListn = 0, clintConnt = 0;
    struct sockaddr_in ipOfServer;
    clintListn = socket(AF_INET, SOCK_STREAM, 0); // creating socket
    memset(&ipOfServer, '0', sizeof(ipOfServer));
    memset(dataSending, '0', sizeof(dataSending));
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY);
    ipOfServer.sin_port = htons(port_num); 		// this is the port number of running server
    bind(clintListn, (struct sockaddr*)&ipOfServer , sizeof(ipOfServer));
    listen(clintListn , 20);

    while(1)
    {
        pthread_t thread_id;
        printf("Hi,Iam the tracker.Some Client hit me\n"); // whenever a request from client came. It will be processed here.
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


}
int main (int argc,char *argv[]){
    sub_downloader(TORRENT_NAME,TRACKER_IP,TRACKER_PORT);
    stringstream strValue;
    strValue << argv[1];
    unsigned int portValue;
    strValue >> portValue;
    cout<<portValue;
    thread upload(share,portValue);
    upload.join();
};
