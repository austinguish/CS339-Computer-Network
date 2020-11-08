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
void sub_downloader(const string fn, string dest_ip, string dest_port){
    cout<<dest_ip<<endl;
    cout<<dest_port<<endl;
    clock_t start = clock();
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
    FILE *fp = fopen(file_name, "wb+");
    if (fp == NULL) {
        printf("File open error");
        return;
    }
    bzero(buffer,BUFFER_SIZE);
    int length = 0;
    long recvlength = 0;
    while((length = recv(CreateSocket,buffer,BUFFER_SIZE,0))>0) {
        if(fwrite(buffer,sizeof(char),length,fp)<length) {
            printf("Failed\n");
            break;
        }
        recvlength+=BUFFER_SIZE;
        bzero(buffer,BUFFER_SIZE);
    }
    fclose(fp);
    cout<<"rev"<<recvlength<<endl;
    printf("Receive done\n");
    close(CreateSocket);
    cout<<"Time cost:"<<clock()-start<<endl;
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

int main (int argc,char *argv[]){
    sub_downloader(TORRENT_NAME,TRACKER_IP,TRACKER_PORT);
    thread download(downloader);
    download.join();
};
