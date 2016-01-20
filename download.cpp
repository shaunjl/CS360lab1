#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sstream>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         400
#define HOST_NAME_SIZE      255
#define TRUE                 1 
#define FALSE                0
#define MAXMSG            1024

int is_number(const char *input)
{
    bool is_num = TRUE;
    int i = 0;
    while (input[i] != '\0'){
        if (!std::isdigit(input[i]))
            is_num = FALSE;
        i++;
    }
    return is_num;
}

int  main(int argc, char* argv[])
{
    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    unsigned toRead = INT_MAX;
    unsigned totalRead = 0;
    char strHostName[HOST_NAME_SIZE];
    char url[HOST_NAME_SIZE];
    int nHostPort;

    int num_download = 1;
    int dflag = FALSE;
    int c = 0;

    if(argc < 4)
        {
        printf("\nUsage: client <options> host-name host-port url\n");
        return 0;
        }
    else
        {
        try
            {
            while ((c = getopt (argc, argv, "c:d")) != -1)
                switch (c)
                    {
                    case 'c':
                        num_download = (int) strtol(optarg, (char **)NULL, 10);
                        break;
                    case 'd':
                        dflag = TRUE;
                        break;
                    case '?':
                        if (optopt == 'c')
                            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                        else
                            fprintf (stderr,
                                   "Unknown option character `\\x%x'.\n",
                                   optopt);
                        return 1;
                    default:
                        abort ();
                    }

            strcpy(strHostName,argv[optind]);
            nHostPort=atoi(argv[optind + 1]);
            strcpy(url, argv[optind + 2]);
            }
            catch (int i)
                {
                    printf("\nUsage: client <options> host-name host-port url\n");
                    return 0;  
                }
        }
    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
       == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
    char *message = (char *)malloc(MAXMSG);
    sprintf(message, "GET %s HTTP/1.1\r\nHOST:%s:%i\r\n\r\n", url, strHostName, nHostPort);
    if(dflag)
        printf("\n%s",message);
    write(hSocket,message,strlen(message));

    // Read until I have got the whole message. Once I have read the full header change the amount to be read

    /* read from socket into buffer
    ** number returned by read() and write() is the number of bytes
    ** read or written, with -1 being that an error occured */
    // int readHeaders = FALSE;
    // while(totalRead < toRead){
    //     memset(pBuffer, 0, BUFFER_SIZE);
    //     nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
    //     if (readHeaders)
    //         totalRead += nReadAmount;
    //     printf("\namount read: %i\n", nReadAmount);
    //     printf("Response: \n%s",pBuffer);
    //     std::stringstream input;
    //     int i;
    //     //search for \r\n\r\n
    //     for(i = 0; i < nReadAmount; i++){
    //         input << pBuffer[i];
    //     }
    //     std::string c_length = "Content-Length: ";
    //     std::size_t found = input.str().find(c_length);
    //     if (found!=std::string::npos){
    //         //get the amount to read
    //         std::stringstream toReadSS;
    //         int j = static_cast<int>(found) + c_length.length();
    //         for(j; j < nReadAmount; j++){
    //             printf("char: %c\n", pBuffer[j]);
    //             toReadSS << pBuffer[j];
    //             if (pBuffer[j + 1] == '\r'){
    //                 break;
    //             }
    //         }
    //         toRead = atoi(toReadSS.str().c_str());
    //         printf("\namount to read: %i\n", toRead);
    //         readHeaders = TRUE;
    //     }
    // }
    int readHeaders = FALSE;
    while(1){
        memset(pBuffer, 0, BUFFER_SIZE);
        nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
        printf(pBuffer);
        if(dflag == TRUE && readHeaders == FALSE){
            std::stringstream headers;
            int i;
            for(i = 0; i < nReadAmount - 1; i++){
                if (pBuffer[i] == '\n' && pBuffer[i+1] == '\n')
                    break;
                headers << pBuffer[i];
            }
            readHeaders = TRUE;
            prinf("headers: %s", headers.str());
        }

        if (nReadAmount == 0){
            break;
        }
    }
    if(close(hSocket) == SOCKET_ERROR)
    {
	printf("\nCould not close socket\n");
    }
    free(message);
}
