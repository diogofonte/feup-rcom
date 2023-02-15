#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "download.h"

int main(int argc, char **argv) {

    if (argc != 2){
        printf("Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }

    struct connection c;
    initConnection(&c);
    if(parseInput(&c, argv[1]) != 0){
        printf("Coudn't parse input\n");
        return -1;
    }

    printf("User: %s\n", c.user);
    printf("Password: %s\n", c.password);
    printf("Host name: %s\n", c.host);
    printf("File path: %s\n", c.urlPath);

    char * ip = (char *)malloc(15);
    char * r = (char *)malloc(1000);
    if(getIP(ip, c.host) != 0){
        printf("Coudn't get IP\n");
        return -1;
    };

    if((c.ctrlfd = createConnection(ip, SERVER_PORT)) < 0){
        printf("Coudn't connect CTRLFD\n");
        return -1;
    }

    if(checkResponse(c.ctrlfd, "220", r) != 0){
        printf("Incorrect response\n");
        return -1;
    }

    if(login(c.ctrlfd, c.user, c.password, c.annonymous) != 0 ){
        printf("Couldn't login\n");
        return -1;
    }

    if(enterPassiveMode(c.ctrlfd, &c) != 0){
        printf("Couldn't enter passive mode\n");
        return -1;
    }

    if(sendCommand(c.ctrlfd, "RETR", c.urlPath, 1)){
        printf("Couldn't send cmd retr\n");
        return -1;
    }

    if (checkResponse(c.ctrlfd, "150", r))
    {
        printf("Wrong response\n");
        return -1;
    }

    if(transfer(c.datafd, c.filename) != 0){
        printf("Couldn't transfer file\n");
        return -1;
    }

    if(checkResponse(c.ctrlfd, "226", r)){
        printf("Incorrect response\n");
        return -1;
    }

    if (close(c.ctrlfd) < 0) {
        printf("Error closing controll socket!\n");
        return -1;
    }

    if (close(c.datafd) < 0) {
        printf("Error closing data socket!\n");
        return -1;
    }
    free(ip);
    free(r);
    return 0;
}
