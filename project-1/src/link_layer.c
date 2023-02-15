// Link layer protocol implementation
#include "link_layer.h"

volatile int STOP = FALSE;
int alarmEnabled = FALSE;
int alarmCount = 0;
int retrasmit = FALSE;

// Alarm function handler
void alarmHandler(int signal){
    alarmEnabled = FALSE;
    alarmCount++;
    printf("Alarm #%d\n", alarmCount);
    retrasmit = TRUE;
}


int llopen(LinkLayer *ll, int role){
    int fd = open(ll->serialPort, O_RDWR | O_NOCTTY);
    if(fd < 0){
        return -1;
    }
    struct termios newtio;

    // Save current port settings
    if(tcgetattr(fd, &oldtio) == -1){
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = ll->timeout *10; // Inter-character timer unused
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if(tcsetattr(fd, TCSANOW, &newtio) == -1){
        perror("tcsetattr");
        exit(-1);
    }

    switch (role) {
    case tx:{
        
        unsigned char * frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
        unsigned char * ua_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
        
        createSUFrame(SET_FRAME, frame, role);

        (void)signal(SIGALRM, alarmHandler);
        alarmCount = 0;
        STOP = FALSE;
        while(alarmCount < ll->nRetransmissions && STOP == FALSE){
            write(fd, frame, S_U_BUF_SIZE);

            printf("SET sent - ");
            printFrame(frame, S_U_BUF_SIZE);
            
            alarm(ll->timeout);

            if(receiveSUFrame(fd, ua_frame, UA, tx) == -1){
                retrasmit = FALSE;
                printf("Failed to receive UA\n");
            } else STOP = TRUE;

            printf("UA received - ");
            printFrame(ua_frame, S_U_BUF_SIZE);
        }
        if(alarmCount == ll->nRetransmissions){
            printf("Exceeded time limit\n");
            exit(-1);
        }
            
        free(ua_frame);
        free(frame);
        break;
    }
    case rx:{
        unsigned char * set_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
        unsigned char * frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));

        if(receiveSUFrame(fd, set_frame, SET, rx) == -1){
            printf("Failed to receive SET\n");
            return -1;
        }

        printf("SET received - ");
        printFrame(set_frame, S_U_BUF_SIZE);
        createSUFrame(UA_FRAME, frame, role);

        write(fd, frame, S_U_BUF_SIZE);
        printf("UA sent - ");
        printFrame(frame, S_U_BUF_SIZE);
        free(set_frame);
        free(frame);
        break;
    }
    default:
        return -1;
        break;
    }

    return fd;
}

int llwrite(const unsigned char *buf, int bufSize, LinkLayer *ll, int port, int role){

    unsigned char * I_frame = (unsigned char *)malloc((bufSize*2 + 6)*sizeof(unsigned char));
    unsigned char * ack_frame = (unsigned char *)malloc(S_U_BUF_SIZE*sizeof(unsigned char));

    int size = createInfoFrame(bufSize, buf, ll->iFrameType, I_frame);

    printf("Sending %d bytes in the data I\n", size);

    printf("Ready to send the I frame\n");

    (void)signal(SIGALRM, alarmHandler);
    alarmCount = 0;
    STOP = FALSE;
    while(alarmCount < ll->nRetransmissions && STOP == FALSE){
        write(port, I_frame, size);
        printf("I sent - ");
        printFrame(I_frame, size);

        alarm(ll->timeout);

        if(receiveSUFrame(port, ack_frame, ACK, role) == -1){
            retrasmit = FALSE;
            printf("Failed to receive acknowlegment\n");
            continue;
        } else STOP = TRUE;

        printf("ACK received - ");
        printFrame(ack_frame, S_U_BUF_SIZE);

        if(ack_frame[2] == C_RR1){
            ll->iFrameType = 1;
        } else if (ack_frame[2] == C_RR0){
            ll->iFrameType = 0;
        }
        else {
            printf("An error has occured. Resend frame\n");
            return -1;
        }
    }

    if(alarmCount == ll->nRetransmissions){
        printf("Exceeded time limit\n");
        exit(-1);
    }
    return bufSize;
}

int llread(unsigned char *packet, LinkLayer *ll, int port, int role){
    printf("Entered ll.read\n");
    printf("I Frame type = %d\n", ll->iFrameType);
    unsigned char * I_frame = (unsigned char *)malloc((MAX_PAYLOAD_SIZE*2 + 6)*sizeof(unsigned char));
    int bytesReceived;
    int numTries = 0;
    while(numTries < ll->nRetransmissions){
        printf("Ready to receive the I frame\n");
        bytesReceived = receiveInfoFrame(port, I_frame);

        if(bytesReceived == -1){
            printf("Failed to receive I frame\n");
            numTries++;
            printf("numtries = %d\n", numTries);
            continue;
        }else numTries = 0;

        printf("I received - ");
        printFrame(I_frame, bytesReceived);

        unsigned char * ack_frame = (unsigned char *)malloc(S_U_BUF_SIZE*sizeof(unsigned char));

        if(I_frame[4] == 1 && I_frame[5] != ll->packetNumber && ll->iFrameType == 1){
            printf("Duplicate packet. Resend I1\n");
            bytesReceived = -1;
            createSUFrame(RR1_FRAME, ack_frame, role);
        } 
        else if(I_frame[4] == 1 && I_frame[5] != ll->packetNumber && ll->iFrameType == 0){
            printf("Duplicate packet. Resend I0\n");
            bytesReceived = -1;
            createSUFrame(RR0_FRAME, ack_frame, role);
        } 
        else if(I_frame[2] == C_I0 && ll->iFrameType == 1){
            printf("Wrong packet. Send I1\n");
            bytesReceived = -1;
            createSUFrame(REJ1_FRAME, ack_frame, role);
        }
        else if (I_frame[2] == C_I1 && ll->iFrameType == 0){
            printf("Wrong packet. Send I0\n");
            bytesReceived = -1;
            createSUFrame(REJ0_FRAME, ack_frame, role);
        }
        else if (I_frame[2] == C_I1){
            printf("Receiving Packet...\n");
            createSUFrame(RR0_FRAME, ack_frame, role);
            ll->iFrameType = 0;
        }
        else if (I_frame[2] == C_I0){
            printf("Receiving Packet...\n");
            createSUFrame(RR1_FRAME, ack_frame, role);
            ll->iFrameType = 1;
        }
        else{
            return -1;
        } 

        write(port, ack_frame, S_U_BUF_SIZE);

        printf("ACK sent - ");
        printFrame(ack_frame, S_U_BUF_SIZE);

        for(int i = 0; i < bytesReceived; i++){
            packet[i] = I_frame[i+4];
        }
        break;
    }

    if(numTries == ll->nRetransmissions){
        printf("Number of tries exceeded\n");
        exit(-1);
    }
    return bytesReceived;
}

int llclose(LinkLayer ll, int port, int role){
    if (role == tx) {
        if(discT(ll, port, role) != 1){
            printf("Failed to disconnect transmitter\n");
            return -1;
        }
    } else if (role == rx) {
        if(discR(ll, port, role) != 1){
            printf("Failed to disconnect receiver\n");
            return -1;
        }
    }

    if (tcsetattr(port, TCSANOW, &oldtio) == -1){
        perror("Couldn't close port\n");
        return -1;
    }

    return close(port);
}

int discT(LinkLayer ll, int port, int role) {
    unsigned char * disc_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
    unsigned char * disc_rcv_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
    unsigned char * ua_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));

    createSUFrame(DISC_FRAME, disc_frame, role);
    (void)signal(SIGALRM, alarmHandler);
    alarmCount = 0;
    STOP = FALSE;
    while(alarmCount < ll.nRetransmissions && STOP == FALSE){
        write(port, disc_frame, S_U_BUF_SIZE);
        alarm(ll.timeout);
        printf("Disc 1 sent - ");
        printFrame(disc_frame, S_U_BUF_SIZE);

        if(receiveSUFrame(port, disc_rcv_frame, DISC, role) == -1){
            printf("Failed to receive DISC 2 from receiver\n");
            return -1;
        } else { STOP = TRUE; }
        printf("Disc 2 received - ");
        printFrame(disc_rcv_frame, S_U_BUF_SIZE);
    }

    if(alarmCount == ll.nRetransmissions){
        printf("Exceeded time limit\n");
        exit(-1);
    }
    createSUFrame(UA_FRAME, ua_frame, role);

    write(port, ua_frame, S_U_BUF_SIZE);

    

    printf("UA sent - ");
    printFrame(ua_frame, S_U_BUF_SIZE);
    return 1;
}

int discR(LinkLayer ll, int port, int role) {
    unsigned char * disc_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
    unsigned char * disc_rcv_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));
    unsigned char * ua_frame = (unsigned char *)malloc(S_U_BUF_SIZE * sizeof(unsigned char));

    if(receiveSUFrame(port, disc_rcv_frame, DISC, role) == -1){
        printf("Failed to receive DISC 1 from transmitter\n");
        return -1;
    }
    printf("Disc 1 received - ");
    printFrame(disc_rcv_frame, S_U_BUF_SIZE);

    createSUFrame(DISC_FRAME, disc_frame, role);
    
    (void)signal(SIGALRM, alarmHandler);
    alarmCount = 0;
    STOP = FALSE;
    while(alarmCount < ll.nRetransmissions && STOP == FALSE){
        write(port, disc_frame, S_U_BUF_SIZE);
        alarm(ll.timeout);
        printf("Disc 2 sent - ");
        printFrame(disc_frame, S_U_BUF_SIZE);

        if(receiveSUFrame(port, ua_frame, UA, role) == -1){
            printf("Failed to receive UA from transmitter\n");
            return -1;
        } else { STOP = TRUE; }
        printf("UA received - ");
        printFrame(ua_frame, S_U_BUF_SIZE);
    }

    if(alarmCount == ll.nRetransmissions){
        printf("Exceeded time limit\n");
        exit(-1);
    }
    return 1;
}

int receiveSUFrame(int fd, unsigned char * frame_buf, TYPE type, int role){
    printf("Receiving Supervison or Unnumbered Frame...\n");
    unsigned char receiver;
    int bytes = 0;
    int byte = 0;
    retrasmit = FALSE;
    STATE state = START;
    do {
        byte = read(fd, &receiver, 1);
        if(retrasmit == TRUE){
            printf("Failed to receive Supervision frame\n");
            return -1;
        }
        if(byte == 0){
            continue;
        }
        bytes++;
        state = machine(receiver, state, type, role);
        if(state == START){
            printf("byte: %d \n", receiver);
            frame_buf = (unsigned char *)malloc(S_U_BUF_SIZE*sizeof(unsigned char));
            bytes = 0;
            continue;
        }
        frame_buf[bytes -1] = receiver;        
    } while(state != STOP_STATE);

    return 1;
}

int receiveInfoFrame(int fd, unsigned char * frame_buf){
    printf("Receiving I frame...\n");
    unsigned char receiver_char;
    int bytes = 0, byte = 0;
    STATE state = START;
    retrasmit = FALSE;
    do {
        byte = read(fd, &receiver_char, 1);
        if(byte == 0){
            return -1;
        }
        bytes++;
        state = machine_info(receiver_char, state, frame_buf);
        if(state == START){
            printf("byte: %d \n", receiver_char);
            frame_buf = (unsigned char *)malloc((MAX_PAYLOAD_SIZE*2 + 6)*sizeof(unsigned char));
            bytes = 0;
        }
    } while(state != STOP_STATE);


    int newLength = byteDestuffing(bytes, frame_buf);

    if(realloc(frame_buf, newLength*sizeof(unsigned char)) == NULL){
        (printf("Realloc failed\n"));
    }

    return newLength;
}