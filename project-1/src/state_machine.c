#include "state_machine.h"

unsigned char c = 0x00;

STATE machine(unsigned char input, STATE state, TYPE type, int role){
    switch(state){
        case START:
            if(input == FLAG){                
                state = FLAG_RECEIVED;
            }
            break;
        case FLAG_RECEIVED:
            if(input == A || (input == A_DISC && (type == UA || type == DISC)))
                state = A_RECEIVED; 
            else if(input == FLAG)
                state = FLAG_RECEIVED;
            else {
                state = START;
            }
            break;
        case A_RECEIVED:
            if(type == SET){
                if(input == C_SET){
                    state = C_RECEIVED;
                    break;
                } 
            } 
            else if(type == UA){
                if(input == C_UA){
                    state = C_RECEIVED;
                    break;
                } 
                if(input == C_UA){
                    state = C_RECEIVED;
                    break;
                }
            }
            else if(type == ACK){
                if(input == C_RR1 || input == C_RR0 || input == C_REJ1 || input == C_REJ0 ){
                    state = C_RECEIVED;
                    c = input;
                    break;
                }
            }
            else if(type == DISC){
                if(input == C_DISC){
                    state = C_RECEIVED;
                    break;
                } 
            }
            if(input == FLAG){
                state = FLAG_RECEIVED;
            } else state = START;
            break;
        case C_RECEIVED:
            if(type == SET){
                if(input == (A^C_SET)){
                    state = BCC_OK;
                    break;
                } 
            } else if(type == UA){
                if(input == (A^C_UA) && role == 0){
                    state = BCC_OK;
                    break;
                }
                if(input == (A_DISC^C_UA) && role == 1){
                    state = BCC_OK;
                    break;
                }
            } else if(type == ACK){
                if(input == (A^c)){
                    state = BCC_OK;
                    break;
                }
            } else if(type == DISC){
                if(role == 0 && input == (A_DISC^C_DISC)){
                    state = BCC_OK;
                    break;
                }
                else if(role == 1 && input == (A^C_DISC)){
                    state = BCC_OK;
                    break;
                }
            }
            if(input == FLAG){
                state = FLAG_RECEIVED;
            } else state = START;

            break;
        case BCC_OK:
            if(input == FLAG){
                state = STOP_STATE;
            } else {
                state = START;
            }
            break;
        case STOP_STATE:
            state = START;
            break;
    }

    return state;
}

STATE machine_info(unsigned char input, STATE state, unsigned char * frame){
    static int i = 0;

    switch(state){
        case START:
            i = 0;
            if(input == FLAG){      
                frame[i] = FLAG;    
                i++;      
                state = FLAG_RECEIVED;
            }
            break;
        case FLAG_RECEIVED:
            if(input == A){
                frame[i++] = A;
                state = A_RECEIVED;
            } else if(input == FLAG){
                break;
            } else {
                state = START;
            }
            break;
        case A_RECEIVED:
            if(input == C_I0){
                frame[i++] = C_I0;
                c = C_I0;
                state = C_RECEIVED;
                break;
            }
            if(input == C_I1){
                frame[i++] = C_I1;
                c = C_I1;
                state = C_RECEIVED;
                break;
            }
            if(input == FLAG){
                state = FLAG_RECEIVED;
            } else state = START;
            break;
        case C_RECEIVED:
            if(input == (A^c)){
                frame[i++] = input;
                state = BCC_OK;
                break;
            }
            if(input == FLAG){
                state = FLAG_RECEIVED;
            } else state = START;
            break;
        case BCC_OK:
            if(input == FLAG){
                state = STOP_STATE;
                frame[i++]=FLAG;
            } else  frame[i++] = input;
            break;

        case STOP_STATE:
            state = START;
            break;
    }

    return state;
}