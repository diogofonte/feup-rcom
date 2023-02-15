#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "definitions.h"

typedef enum{
    START, 
    FLAG_RECEIVED,
    A_RECEIVED,
    C_RECEIVED,
    BCC_OK,
    STOP_STATE
} STATE; //Possible states of the state machine

typedef enum{
    SET,
    UA,
    ACK,
    DISC
} TYPE; //Possible types of frames for the Supervision frames

/**
 * Receives an input and updates the state of the state machine according to the type of frame and role
 * Returns the current state of the state machine
 * Arguments:
 *  - input - input the state machine receives
 *  - state - current state of the state machine
 *  - type - frame type
 *  - role - application layer role ("tx" - 0 or "rx" - 1)
*/
STATE machine(unsigned char input, STATE state, TYPE type, int role);

/**
 * Receives an input from the I frame being received, updates the state of the state machine and updates the buffer of the frame
 * Returns the current state of the state machine
 * Arguments:
 *  - input - input the state machine receives
 *  - state - current state of the state machine
 *  - frame - current I frame
*/
STATE machine_info(unsigned char input, STATE state, unsigned char * frame);

#endif
