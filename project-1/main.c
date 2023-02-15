#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "application_layer.h"

// Arguments:
//   $1: /dev/ttySxx
//   $2: read | write or tx | rx pelo original
//   $3: filename
int main(int argc, char *argv[]){
    
    if(argc < 4){
        printf("Usage: %s /dev/ttySxx tx|rx filename\n", argv[0]);
        exit(1);
    }
   
    struct timeval start, end;
    const char *serialPort = argv[1];   // /dev/ttyS0
    const char *role       = argv[2];   // read or write
    const char *filename   = argv[3];

    // check if the serial port is valid
    if((strcmp(PORT_0, serialPort)!=0 && strcmp(PORT_1, serialPort)!=0)){
        printf("Invalid serial port\n");
        exit(1);
    }
    // verify if role is valid
    if(strcmp("tx", role)!= 0 && strcmp("rx", role)!=0){
    	printf("Invalid role\n");
        exit(1);
    }

    printf("Starting link-layer protocol application\n"
           "  - Serial port: %s\n"
           "  - Role: %s\n"
           "  - Baudrate: %d\n"
           "  - Number of tries: %d\n"
           "  - Timeout: %d\n"
           "  - Filename: %s\n",
           serialPort,
           role,
           BAUDRATE,
           N_TRIES,
           TIMEOUT,
           filename);

    gettimeofday(&start, NULL);
    applicationLayer(serialPort, role, BAUDRATE, N_TRIES, TIMEOUT, filename);
    gettimeofday(&end, NULL);

    long seconds = (end.tv_sec - start.tv_sec);
    long micro = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

    printf("Time elapsed: %ld micro seconds\n", micro);  

    return 0;
}
