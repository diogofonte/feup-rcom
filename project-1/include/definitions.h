#ifndef PROJETO_1_DEFINITIONS_H
#define PROJETO_1_DEFINITIONS_H

//#define BAUDRATE 9600
#define N_TRIES 3
#define TIMEOUT 4
#define PORT_0 "/dev/ttyS10"
#define PORT_1 "/dev/ttyS11"

#define FLAG     0x7E
#define ESC      0x7D
#define A        0x03
#define A_DISC   0x01
#define C_SET    0x03
#define C_UA     0x07
#define C_RR0    0x05
#define C_RR1    0xB5
#define C_REJ0   0x01
#define C_REJ1   0x81
#define C_I0     0x00
#define C_I1     0xB0
#define C_DISC   0x0B

#define FALSE 0
#define TRUE 1
// SIZE of maximum acceptable payload.
// Maximum number of bytes that application layer should send to link layer
#define MAX_PAYLOAD_SIZE 1000


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define TRANSMITTER 0
#define RECEIVER    1


#include <errno.h>
// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define BUF_SIZE 256 
#define S_U_BUF_SIZE 5 //Size of frames SET, UA, DISC, RR and REJ

typedef enum {
    tx,
    rx
} Role;

#endif //PROJETO_1_DEFINITIONS_H
