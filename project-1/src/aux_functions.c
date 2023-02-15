#include "aux_functions.h"

int byteStuffing(int size, const unsigned char* data, unsigned char* I_frame, unsigned char bcc2){
    int length = 4;
    unsigned char byte;
    for(int i = 0; i < size ; i++) {
        byte = data[i];
        if(byte == ESC){
            I_frame[length++] = ESC;
            I_frame[length] = ESC^0x20;
        } else if (byte == FLAG){
            I_frame[length++] = ESC;
            I_frame[length] = FLAG^0x20;
        } else I_frame[length] = byte;
        length++;
    }

    if(bcc2 == ESC){
            I_frame[length++] = FLAG;
            I_frame[length] = ESC^0x20;
    } else if (byte == FLAG){
            I_frame[length++] = ESC;
            I_frame[length] = FLAG^0x20;
    } else I_frame[length] = byte;

    return length;
}

int byteDestuffing(int size, unsigned char * I_frame){
    unsigned char aux_buf[size];

    for(int i = 0; i < size ; i++) {
        aux_buf[i] = I_frame[i];
    }

    unsigned char b;
    int wasESC = FALSE;
    int length = 4;

    for(int i = 4; i < size; i++){
        b = I_frame[i];
        if(b == ESC){
            wasESC = TRUE;
            continue;
        }
        else if (wasESC){
            if(b == (FLAG^0x20)){
                I_frame[length] = FLAG;
            } else if (b == (ESC^0x20)){
                I_frame[length] = ESC;
            }
            length++;
            wasESC = FALSE;
        }
        else{
            I_frame[length] = aux_buf[i];
            length++;
        }
    }

    return length;
}
   
void createSUFrame(FRAME_TYPE type, unsigned char * frame, int role){
    frame[0] = FLAG;
    frame[1] = A;
    switch (type) {
    case SET_FRAME:
        frame[2] = C_SET;
        frame[3] = A^C_SET;
        break;
    
    case UA_FRAME:
        if(role == 0){
            frame[1] = A_DISC;
            frame[2] = C_UA;
            frame[3] = A_DISC^C_UA;
        }
        else if(role == 1){
            frame[2] = C_UA;
            frame[3] = A^C_UA;
        }
        break;

    case RR1_FRAME:
        frame[2] = C_RR1;
        frame[3] = A^C_RR1;
        break;

    case RR0_FRAME:
        frame[2] = C_RR0;
        frame[3] = A^C_RR0;
        break;

    case REJ1_FRAME:
        frame[2] = C_REJ1;
        frame[3] = A^C_REJ1;
        break;

    case REJ0_FRAME:
        frame[2] = C_REJ0;
        frame[3] = A^C_REJ0;
        break;

    case DISC_FRAME:
        if(role == 0){
            frame[2] = C_DISC;
            frame[3] = A^C_DISC;
        }
        else if(role == 1){
            frame[1] = A_DISC;
            frame[2] = C_DISC;
            frame[3] = A_DISC^C_DISC;
        }
        
        break;
    default:
        break;
    }

    frame[4] = FLAG;
}

int createInfoFrame(int size, const unsigned char *data, int iFrameType, unsigned char * I_frame){
    int length = 0;
    unsigned char bcc2 = 0x00;
    I_frame[0] = FLAG;
    I_frame[1] = A;
    if(iFrameType == 0){
        I_frame[2] = C_I0;
        I_frame[3] = A^C_I0;
    } else {
        I_frame[2] = C_I1;
        I_frame[3] = A^C_I1;
    }
    for (int i = 0; i < size; i++){
        bcc2 = bcc2^data[i];
    }
    length = byteStuffing(size, data, I_frame, bcc2);

    I_frame[length++] = FLAG;

    if(realloc(I_frame, length*sizeof(unsigned char)) == NULL){
        (printf("Realloc failed\n"));
    }

    return length;
}

void printFrame(unsigned char * frame, int size){
    printf("Frame: ");
    for(int i = 0; i < size; i++){
        printf("%u ", frame[i]);    
    }
    printf("\n\n");
}
