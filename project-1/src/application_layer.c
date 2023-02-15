// Application layer protocol implementation

#include "application_layer.h"
#include <sys/stat.h>

ApplicationLayer al;
FileInfo file;
LinkLayer ll;

void applicationLayer(const char *serialPort, const char *role, int baudRate, int nTries, int timeout, const char *filename){

    // incluir os dados da estrutura LinkLayer
    for(int i = 0; i < 50 ; i++) {
        ll.serialPort[i] = serialPort[i];
    }
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;
    ll.iFrameType = 0;

    printf("Entered app layer\n");

    if(strcmp(role, "tx") == 0){
        al.status = 0;
        memcpy(&file.name, &filename, strlen(filename));
        for(int i = 0; i < strlen(filename); i++){
            file.name[i] = filename[i];
        }
        transmitter_app();

    } else if(strcmp(role, "rx") == 0){
        al.status = 1;
        
        receiver_app();
    }

}

///////////////////////////////////////////////////////////////////////////////////////

void transmitter_app(){
    // open file
    file.file = fopen(file.name, "r");
    if (file.file == NULL){
        printf("Cannot open file\n");
        return;
    }
    // get file size
    struct stat st;
    stat(file.name, &st);
    file.size = st.st_size;

    // open serial port
    al.port = llopen(&ll, al.status);    
    if(al.port <= 0){
        perror("Cannot open serial port\n");
        exit(-1);
    }
    printf("Conection established\n");

    unsigned char * controlPacket = (unsigned char *)malloc((MAX_PAYLOAD_SIZE+4)*sizeof(unsigned char));
    int controlPacketSize = createControlPacket(controlPacket);

    printf("Sending Control packet\n"); 

    
    if(llwrite(controlPacket, controlPacketSize, &ll, al.port, al.status) == -1){
        perror("Control Packet not sent\n");
        exit(-1);
    }
    printf("Control Packet sent!\n\n");                                
    printf("Sending file...\n");

    size_t bytesRead = 0, bytesWritten = 0;
    ll.packetNumber = 0;
    int size = 0;
    while(bytesRead < file.size){
        unsigned char * dataPacket = (unsigned char *)malloc((MAX_PAYLOAD_SIZE+4)*sizeof(unsigned char));
        if(bytesRead + MAX_PAYLOAD_SIZE < file.size){
            size = MAX_PAYLOAD_SIZE;
        } else size = file.size - bytesRead;

        unsigned char * aux_buf = (unsigned char *)malloc(size*sizeof(unsigned char));
        
        if(fread(aux_buf, size, 1, file.file) != 1){
            perror("Couldn't read from file \n");
            exit(-1);
        }
        int dataSize = createDataPacket(dataPacket, ll.packetNumber, size, aux_buf);
        printf("Sending Packet...\n\n");
        bytesWritten = llwrite(dataPacket, dataSize, &ll, al.port, al.status);
        printf("Packet sent!\n");

        bytesRead += bytesWritten - 4; // 4 bytes of header
        ll.packetNumber = (ll.packetNumber+1)%256;

        free(dataPacket);
        free(aux_buf);
    }

    controlPacket[0] = 3; //end
    printf("Sending control packet 2\n"); 
    if(llwrite(controlPacket, controlPacketSize, &ll, al.port, al.status) < 0){
        perror("Control Packet 2 not sent");
        exit(-1);
    }
    printf("Control Packet sent!\n\n");

    printf("Finished transmission\n");

    // close serial port
    if(llclose(ll, al.port, al.status) < 0){
        perror("Cannot close serial port\n");
        exit(-1);
    }
    printf("Connection terminated\n");

    // close file
    if(fclose(file.file) != 0){
        perror("Cannot close file\n");
        exit(-1);
    }

    free(controlPacket);
}

/////////////////////////////////////////////////////////////////////////////////////////

void receiver_app(){
    // open serial port
    al.port = llopen(&ll, al.status);
    if(al.port <= 0){
        perror("Cannot open serial port\n");
        exit(-1);
    }
    printf("Connection established\n");

    //LER CONTROL PACKET
    unsigned char * controlPacket = (unsigned char *)malloc((MAX_PAYLOAD_SIZE+4)*sizeof(unsigned char));

    printf("Ready to receive the control packet\n");
    
    if(llread(controlPacket, &ll, al.port, al.status) == -1){
        perror("Control Packet not received\n");
        exit(-1);
    }

    printf("Control packet received!\n");

    if(checkControlPacket(controlPacket, 2) == -1){
        perror("Wrong control packet\n");
        exit(-1);
    }

    printf("Creating the file\n");
    // create/open file
    file.file = fopen("penguin-received.gif", "w");
    if (file.file == NULL){
        printf("Cannot open file\n");
        return;
    }

    printf("Start receiving file...\n");

    size_t bytesRead = 0, bytesWritten = 0 ;
    ll.packetNumber = 0;
    int size = 0;
    while(bytesRead < file.size){
        unsigned char * dataPacket = (unsigned char *)malloc((MAX_PAYLOAD_SIZE+4)*sizeof(unsigned char));
        if(bytesRead + MAX_PAYLOAD_SIZE < file.size){
            size = MAX_PAYLOAD_SIZE;
        } else size = file.size - bytesRead;
        unsigned char * data = (unsigned char *)malloc(size*sizeof(unsigned char));

        printf("Reading Packet...\n");
        bytesWritten = llread(dataPacket, &ll, al.port, al.status);
        printf("Packet read!\n");

        if(bytesWritten == -1){
            continue;
        }

        if(checkDataPacket(dataPacket, data, ll.packetNumber) == -1){
            printf("Please resend Data Packet %d\n", ll.packetNumber);
            continue;
        }

        if(fwrite(data, size, 1, file.file) != 1){
            perror("Couldn't write to file \n");
            exit(-1);
        }

        bytesRead += bytesWritten - 4 - 5; // 4 bytes of header
        
        ll.packetNumber = (ll.packetNumber+1)%256;

        free(dataPacket);
    }
    
    printf("Ready to receive the control packet 2\n");
    
    if(llread(controlPacket, &ll, al.port, al.status) == -1){
        perror("Control Packet 2 not received\n");
        exit(-1);
    }

    printf("Control packet 2 received!\n");

    if(checkControlPacket(controlPacket, 3) == -1){
        perror("Wrong control packet 2\n");
        exit(-1);
    }

    printf("Transmission ended successfully\n");

    // close serial port
    if(llclose(ll, al.port, al.status) < 0){
        perror("Cannot close serial port\n");
        exit(-1);
    }
    printf("Connection ended successfully\n");

    // close file
    if(fclose(file.file) != 0){
        perror("Cannot close file\n");
        exit(-1);
    }

    free(controlPacket);
}


int createControlPacket (unsigned char * controlPacket){
    controlPacket[0] = 2; //start
    controlPacket[1] = 0;

    int length = file.size, size = 0;
    printf("length = %d\n", length);

    while (length > 0)
    {
        int rest = length % 256;
        int div = length / 256;
        size++;

        for (unsigned int i = 2 + size; i > 3; i--)
            controlPacket[i] = controlPacket[i - 1];

        controlPacket[3] = (unsigned char)rest;

        length = div;
    }
    printf("size = %d\n", size);
    controlPacket[2] = size;
    memcpy(&controlPacket[3], &file.size, size);
    
    controlPacket[3+size] = 1;
    controlPacket[4+size] = strlen(file.name);
    memcpy(&controlPacket[5+size], &file.name, controlPacket[4+size]);

    return controlPacket[4+size] + 5 + size;
}

int createDataPacket(unsigned char * dataPacket, int n, int size, unsigned char * data){
    dataPacket[0] = 1;
    dataPacket[1] = n;
    dataPacket[2] = size / 256;
    dataPacket[3] = size % 256;
    int i;
    for(i = 0; i < size; i++){
        dataPacket[i+4] = data[i];
    }
    return i+4;
}

int checkControlPacket (unsigned char * controlPacket, int role){
    int sizeLength;
    if((int)controlPacket[0] != role)
        return -1;
    if(controlPacket[1] == 0){
        sizeLength = (int)controlPacket[2];
        memcpy(&file.size, &controlPacket[3], sizeLength);
        printf("file.size = %d\n", file.size);
    } else return -1;
    if(controlPacket[3 + sizeLength] == 1){
        int sizeName = (int)controlPacket[4+sizeLength];
        memcpy(&file.name, &controlPacket[5+sizeLength], sizeName);
        printf("file.name = %s\n", file.name);
    } else return -1;
    return 1;
}

int checkDataPacket (unsigned char * dataPacket, unsigned char * data, int sequenceNumber){
    if (dataPacket[0] != 1)
        return -1;
    if(sequenceNumber != (int)dataPacket[1]){
        printf("Different sequence number\n");
        return -1;
    }
    
    int size = 256 * (int)dataPacket[2] + (int)dataPacket[3];

    for (int i = 0; i < size; i++){
        data[i] = dataPacket[i + 4];
    }
    
    return 1;
}