#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>

#define SERVER_PORT 21

struct connection{
    char* user;
    char* password;
    char* host;
    char* urlPath;
    char* filename;
    int annonymous;
    int ctrlfd;
    int datafd;
};

void initConnection (struct connection *connection);
int parseInput(struct connection *connection, char* input);
int getIP(char* ip, char* host);
int createConnection(char* addr, int port);
int checkResponse(int sockfd, char * expectedResponse, char* response);
int login(int sockfd, char * username, char * password, int annonymous);
int sendCommand(int sockfd, char * command, char* args, int hasArg);
int enterPassiveMode(int sockfd, struct connection *connection);
int transfer(int sockfd, char * name);

#endif
