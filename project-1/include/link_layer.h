// Link layer header.
// NOTE: This file must not be changed.

#ifndef _LINK_LAYER_H_
#define _LINK_LAYER_H_

#include "definitions.h"
#include "aux_functions.h"

struct termios oldtio;

typedef struct {
    char serialPort[50];
    int baudRate;
    int nRetransmissions;
    int timeout;
    int iFrameType;
    int packetNumber;
} LinkLayer;

/**
 * Handles the alarm signal
 * Arguments:
 *  - signal - signal received
 */
void alarmHandler(int signal);

/**
 * Establishes a connection using "serialPort" parameter of the LinkLayer
 * Returns the file descriptor on success or -1 otherwise
 * Arguments:
 *  - ll - link layer struct that contains information such as the number of retransmissions, serial port and timeout
 *  - role - application layer role ("tx" - 0 or "rx" - 1)
*/
int llopen(LinkLayer *ll, int role);

/**
 * Sends the data in the buf with size bufSize and receives an acknowlegment
 * Returns the number of bytes written, or -1 on error
 * Arguments:
 *  - buf - buffer with the data that should be sent
 *  - bufSize - size of buf
 *  - ll - link layer struct that contains the information of the number of retransmissions, type of I frame (I0 or I1) and timeout
 *  - port - serial port
 *  - role - application role ("tx" - 0 or "rx" - 1)
*/
int llwrite(const unsigned char *buf, int bufSize, LinkLayer *ll, int port, int role);

/**
 * Receives the data packet and send an acknowlegment
 * Returns the number of bytes read, or -1 on error
 * Arguments:
 *  - packet - buffer that will contain the packet being received
 *  - ll - link layer struct that contains the information of the number of retransmissions, type of I frame (I0 or I1), timeout and packet number
 *  - port - serial port
 *  - role - application role ("tx" - 0 or "rx" - 1)
*/
int llread(unsigned char *packet, LinkLayer *ll, int port, int role);

/**
 * Closes the previously established connection
 * Returns 1 on success or -1 otehrwise
 * Arguments:
 *  - ll - link layer struct that contains the information of the number of retransmissions, type of I frame (I0 or I1) and timeout
 *  - port - serial port
 *  - role - application role ("tx" - 0 or "rx" - 1)
*/
int llclose(LinkLayer ll, int port, int role);

/**
 * Disconnects the receiver
 * Returns 1 on success or -1 otehrwise
 * Arguments:
 *  - ll - link layer struct that contains the information of the number of retransmissions and timeout
 *  - port - serial port
 *  - role - application role ("tx" - 0 or "rx" - 1)
*/
int discR(LinkLayer ll, int port, int role);

/**
 * Disconnects the transmitter
 * Returns 1 on success or -1 otehrwise
 * Arguments:
 *  - ll - link layer struct that contains the information of the number of retransmissions and timeout
 *  - port - serial port
 *  - role - application role ("tx" - 0 or "rx" - 1)
*/
int discT(LinkLayer ll, int port, int role);

/**
 * Functions that receives and verifies the supervision and unnumbered frames (SET, UA, RR, REJ, DISC)
 * Returns 1 on success or -1 otherwise
 * Arguments:
 *  - fd - serial port
 *  - frame_buf - buffer that will contain the frame received
 *  - type - type of the frame (SET, UA, ACK, DISC)
 *  - role - application role ("tx" - 0 or "rx" - 1)
*/
int receiveSUFrame (int fd, unsigned char * frame_buf, TYPE type, int role);

/**
 * Functions that receives and verifies the I frames (I0 or I1)
 * Returns 1 on success or -1 otherwise
 * Arguments:
 *  - fd - serial port
 *  - frame_buf - buffer that will contain the frame received
*/
int receiveInfoFrame(int fd, unsigned char * frame_buf);

#endif // _LINK_LAYER_H_
