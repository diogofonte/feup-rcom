#ifndef _APPLICATION_LAYER_H_
#define _APPLICATION_LAYER_H_

#include "definitions.h"
#include "link_layer.h"

typedef struct {
  int size;        // file size
  char name[255];  // file name
  FILE* file;
} FileInfo;

typedef struct {
  int port;          // serial port
  int status;      // rx | tx
} ApplicationLayer;

/**
 * Application layer main function
 * Initiates the transmitter application and the receiver application according to the role (tx and rx, respectively)
 * Arguments:
 *  - serialPort: Serial port name (e.g., /dev/ttyS0).
 *  - role: Application role {"tx", "rx"}
 *  - baudrate: Baudrate of the serial port.
 *  - nTries: Maximum number of frame retries.
 *  - timeout: Frame timeout.
 *  - filename: Name of the file to send / receive.
*/
void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename);

/**
 * Function that will call the necessary functions to send the file to the receiver.
*/
void transmitter_app();

/**
 * Function that will call the necessary functions to receive the file from the transmitter.
*/
void receiver_app();

/**
 * Creates an application data packet with the given data
 * Returns the size of the packet
 * Arguments:
 *  - dataPacket - buffer that will contain the data packet being created
 *  - n - sequence number
 *  - size - number of octets of the data field
 *  - data - buffer with the data
*/
int createDataPacket(unsigned char * dataPacket, int n, int size, unsigned char * data);

/**
 * Creates an application control packet
 * Returns the size of the packet
 * Arguments:
 *  - controlPacket - buffer that will contain the control packet being created
*/
int createControlPacket (unsigned char * controlPacket);

/**
 * Checks if the control packet was received correctly
 * Returns 1 with the packet is correct or -1 otherwise
 * Arguments:
 *  - controlPacket - buffer with control packet to be checked
 *  - role - control field (2(start) or 3(end))
*/
int checkControlPacket (unsigned char * controlPacket, int role);

/**
 * Checks if the data packet was received correctly and extracts the data from it
 * Returns 1 with the packet is correct or -1 otherwise
 * Arguments:
 *  - dataPacket - buffer with the data packet to be checked
 *  - data - buffer that will be filled with the data from the data packet
 *  - sequenceNumber - sequence number that the data packet should have
*/
int checkDataPacket (unsigned char * dataPacket, unsigned char * data, int sequenceNumber);


#endif // _APPLICATION_LAYER_H_
