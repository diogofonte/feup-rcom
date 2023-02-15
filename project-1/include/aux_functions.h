#ifndef AUX_FUNCTIONS_H_
#define AUX_FUNCTIONS_H_

#include "definitions.h"
#include "state_machine.h"

typedef enum{
    SET_FRAME,
    UA_FRAME,
    RR1_FRAME,
    RR0_FRAME,
    REJ1_FRAME,
    REJ0_FRAME,
    DISC_FRAME
} FRAME_TYPE;

/**
 * Performs byte stuffing on the data and bcc2
 * Returns the current length of the I frame
 * Arguments:
 *  - size - size of the data buffer
 *  - data - buffer with the data to be stuffed
 *  - I_frame - I frame that will contain the stuffed data 
 *  - bcc2 - bcc2 byte that might also suffer stuffing
*/
int byteStuffing(int size, const unsigned char* data, unsigned char* I_frame, unsigned char bcc2);

/**
 * Performs destuffing on the I frame
 * Returns the new length of the I frame
 * Arguments:
 *  - size - size of the I frame
 *  - I_frame - buffer with the I frame
*/
int byteDestuffing(int size, unsigned char * I_frame);

/**
 * Creates the Suppervision (or Unnumbered) frame according to the type and role
 * Arguments:
 *  - type - type of the frame
 *  - frame - buffer that will contain the finished frame
 *  - role - application layer role ("tx" - 0 or "rx" - 1)
*/
void createSUFrame(FRAME_TYPE type, unsigned char * frame, int role);

/**
 * Creates the I frame with the given data 
 * Returns the length of the I frame
 * Arguments:
 *  - size - size of the data buffer
 *  - data - buffer with the data for the I frame
 *  - iFrameType - Type of I frame (0 or 1)
 *  - I_frame - buffer that will contain the finished I frame
*/
int createInfoFrame(int size, const unsigned char *data, int iFrameType, unsigned char * I_frame);

/**
 * Prints the frame given
 * Arguments:
 *  - frame - buffer with the frame to be printed
 *  - size - size of the frame
*/
void printFrame(unsigned char * frame, int size);

#endif //AUX_FUNCTIONS_H