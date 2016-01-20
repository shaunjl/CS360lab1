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

    int num_download = 0;
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

    int readHeaders = FALSE;
    int justReadHeaders = FALSE;
    std::stringstream headers;
    std::stringstream body;
    if(num_download == 0){
        while(1){
            memset(pBuffer, 0, BUFFER_SIZE);
            nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
            int i;
            if(readHeaders == FALSE){
                for(i = 0; i < nReadAmount - 4; i++){
                    if (pBuffer[i] == '\r' && pBuffer[i + 1] == '\n' && pBuffer[i+2] == '\r' && pBuffer[i+3] == '\n')
                        break;
                    headers << pBuffer[i];
                }
                readHeaders = TRUE;
                justReadHeaders = TRUE;
            }
            if(justReadHeaders){
                i += 4;
                justReadHeaders = FALSE;
            } else {
                i = 0;
            }
            for(i; i < nReadAmount; i++)
                body << pBuffer[i];
            if (nReadAmount == 0){
                break;
            }
        }
        if(dflag){
            printf(headers.str().c_str());
            printf("\n\n");
        }
        printf(body.str().c_str());
    }

    int num_success = 0;
    int i;
    for(i = 0; i < num_download; i++){
        while(1)
        {
            memset(pBuffer, 0, BUFFER_SIZE);
            nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
            if (nReadAmount == -1){
                break;
            }
            if (nReadAmount == 0){
                num_success++;
                break;
            }
        }
    }

    if(num_download > 0)
        printf("Successful Downloads: %i\n", num_success);

    if(close(hSocket) == SOCKET_ERROR)
    {
	printf("\nCould not close socket\n");
    }
    free(message);
}
