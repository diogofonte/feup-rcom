#include "download.h"

void initConnection(struct connection *connection)
{
    connection->user = NULL;
    connection->password = NULL;
    connection->host = NULL;
    connection->urlPath = NULL;
    connection->filename = NULL;
    connection->annonymous = -1;
    connection->ctrlfd = -1;
    connection->datafd = -1;
}

int parseInput(struct connection *connection, char *input)
{
    const char at[2] = "@";
    const char colon[2] = ":";
    const char slash[2] = "/";
    const char end[2] = "\0";
    char *token;
    char *rest;

    // ftp://[<user>:<password>@]<host>/<url-path>
    printf("Parsing input...\n\n");

    // check ftp
    token = strtok_r(input, colon, &rest); // token should be ftp

    if (token == NULL)
    {
        printf("Wrong input.\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }
    else if (strcmp(token, "ftp") != 0)
    {
        printf("Protocol name should be ftp\n\n");
        return -1;
    }
    // user and password
    char *aux = (char *)malloc((sizeof(char) * strlen(rest)));
    strcpy(aux, rest);
    token = strtok(aux, at);

    if (token == NULL || token[0] != '/' || token[1] != '/')
    {
        printf("Wrong input (protocol name).\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }
    if (strcmp(token, rest) == 0)
    { // the user is annonymous
        connection->annonymous = 1;
        connection->user = "anonymous";
    }
    else
    {
        // user
        connection->annonymous = 0;
        token = strtok_r(rest, colon, &rest); // gets the user
        if (token == NULL)
        {
            printf("Wrong input (user).\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
            return -1;
        }
        else if (strlen(token) == 0)
        {
            printf("No username.\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
            return -1;
        }
        connection->user = (char *)malloc((sizeof(char) * strlen(token)) + 1 - 2);
        strcpy(connection->user, &token[2]);

        // password
        token = strtok_r(rest, at, &rest); // gets the user
        if (token == NULL)
        {
            printf("Wrong input (password).\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
            return -1;
        }
        else if (strlen(token) == 0)
        {
            printf("No password.\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
            return -1;
        }

        connection->password = (char *)malloc((sizeof(char) * strlen(token)) + 1);
        strcpy(connection->password, token);
    }
    // host
    token = strtok_r(rest, slash, &rest);
    if (token == NULL)
    {
        printf("Wrong input (host).\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }
    else if (strlen(token) == 0)
    {
        printf("No host.\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }

    connection->host = (char *)malloc((sizeof(char) * strlen(token)) + 1);
    strcpy(connection->host, token);

    // url path
    token = strtok_r(rest, end, &rest);
    if (token == NULL)
    {
        printf("Wrong input (url).\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }
    else if (strlen(token) == 0)
    {
        printf("No url.\nUsage: ftp://[<user>:<password>@]<host>/<url-path>\n\n");
        return -1;
    }
    
    connection->urlPath = (char *)malloc((sizeof(char) * strlen(token)) + 1);
    strcat(connection->urlPath, "/");
    strcat(connection->urlPath, token);

    char* name = strrchr(connection->urlPath, '/');
    printf("%s\n", name);

    connection->filename = (char *)malloc((sizeof(char) * strlen(name)) + 1);
    
    if (name != NULL)
    {
        strcpy(connection->filename, name+1);
    }

    free(aux);

    return 0;
}

int getIP(char *ip, char *host)
{
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname()");
        return -1;
    }
    strcpy(ip, inet_ntoa(*((struct in_addr *)h->h_addr)));
    printf("IP Address : %s\n\n", inet_ntoa(*((struct in_addr *)h->h_addr)));
    return 0;
}

int createConnection(char *addr, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    // server address handling
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(addr);
    server_addr.sin_port = htons(port);

    // open a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        return -1;
    }
    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect()");
        return -1;
    }

    return sockfd;
}

int checkResponse(int sockfd, char *expectedResponse, char *response)
{
    // Read response
    printf("Receiving response...\n");
    FILE *s = fdopen(sockfd, "r");
    char *string = (char *)malloc(1000);
    do
    {
        memset(string, 0, 1000);
        string = fgets(string, 1000, s);
        printf("%s", string);
    } while (!('1' <= string[0] && string[0] <= '5') || string[3] != ' ');
    
    // Check response
    if (string[0] != expectedResponse[0] || string[1] != expectedResponse[1] || string[2] != expectedResponse[2])
    {
        printf("Error in the Response\n");
        printf("Expected:%s\nReceived:%s\n", expectedResponse, string);
        return -1;
    }

    strcpy(response, string);

    free(string);
    printf("\n");
    return 0;
}

int login(int sockfd, char *username, char *password, int annonymous)
{
    printf("Login...\n");
    char *r = (char *)malloc(1000);

    int hasArg = annonymous ^ 1;
    if (sendCommand(sockfd, "USER", username, 1) != 0)
    {
        printf("Error sending username\n");
        return -1;
    }

    if (checkResponse(sockfd, "331", r) != 0)
    {
        return -1;
    }

    if (sendCommand(sockfd, "PASS", password, hasArg) != 0)
    {
        printf("Error sending password\n");
        return -1;
    }

    if (checkResponse(sockfd, "230", r) != 0)
    {
        return -1;
    }

    free(r);

    return 0;
}

int sendCommand(int sockfd, char *command, char *args, int hasArg)
{
    char *cmd = (char *)malloc(1000);

    strcpy(cmd, command);

    if (hasArg == 1)
    {
        strcat(cmd, " ");
        strcat(cmd, args);
    }
    strcat(cmd, "\r\n");

    printf("Sending command...\n");
    printf("%s\n", cmd);
    if (write(sockfd, cmd, strlen(cmd)) != strlen(cmd))
    {
        return -1;
    }

    free(cmd);
    return 0;
}

int enterPassiveMode(int sockfd, struct connection *connection)
{
    char *r = (char *)malloc(1000);
    if (sendCommand(sockfd, "PASV", NULL, 0) != 0)
    {
        printf("Error entering passive mode\n");
        return -1;
    }

    if (checkResponse(sockfd, "227", r))
    {
        return -1;
    }

    int ip_arr[4], port_arr[2];
    sscanf(r, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
           &ip_arr[0], &ip_arr[1], &ip_arr[2], &ip_arr[3], &port_arr[0], &port_arr[1]);

    char *ip = (char *)malloc(15);
    int port;

    sprintf(ip, "%d.%d.%d.%d", ip_arr[0], ip_arr[1], ip_arr[2], ip_arr[3]);
    port = 256 * port_arr[0] + port_arr[1];

    if ((connection->datafd = createConnection(ip, port)) < 0)
    {
        printf("Couldn't connect to data fd\n");
        return -1;
    }

    free(r);

    return 0;
}

int transfer(int sockfd, char * name){
    FILE* filefd;

    filefd = fopen(name, "w");
    if(filefd == NULL){
        printf("Error opening file\n");
        return -1;
    }

    char* buf = (char *)malloc(2000);
    int bytes;

    printf("Downloading %s...\n", name);
    while((bytes = read(sockfd, buf, sizeof(buf)))){
        if(bytes < 0){
            printf("Error reading from data socket\n");
            return -1;
        }
        if((bytes = fwrite(buf,1 , bytes, filefd)) < 0){
            printf("Error writing data to file\n");
            return -1;
        }
    }

    if(fclose(filefd) < 0){
        printf("Error closing file\n");
        return -1;
    }

    return 0;
}