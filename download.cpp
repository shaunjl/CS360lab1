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
#define BUFFER_SIZE         256
#define HOST_NAME_SIZE      255
#define TRUE                 1 
#define FALSE                0
#define MAXMSG            1024

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
        while ((c = getopt (argc, argv, "c:d")) != -1)
            switch (c)
              {
              case 'c':
                printf("c is called with value %s", optarg);

                num_download = (int) strtol(optarg, (char **)NULL, 10);
                break;
              case 'd':
                printf("d is called");
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
        printf("Num Download: %i\n", num_download);
        if(dflag == TRUE)
            printf("DFlag set\n");
        strcpy(strHostName,argv[optind]);
        printf ("host name: %s\n", strHostName);
        nHostPort=atoi(argv[optind + 1]);
        printf ("port: %i\n", nHostPort);
        strcpy(url, argv[optind + 2]);
        printf ("url: %s\n", url);
      }

    printf("\nMaking a socket");
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

    printf("\nConnecting to %s (%X) on port %d",strHostName,nHostAddress,nHostPort);

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
       == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
    char *message = (char *)malloc(MAXMSG);
    sprintf(message, "GET %s HTTP/1.1\r\nHOST:%s:%i\r\n\r\n", url, strHostName, nHostPort);
    printf("Message:\n%s\n",message);
    write(hSocket,message,strlen(message));

    // Read until I have got the whole message. Once I have read the full header change the amount to be read

    /* read from socket into buffer
    ** number returned by read() and write() is the number of bytes
    ** read or written, with -1 being that an error occured */
    while(totalRead < toRead){
        printf("Iteration");
        memset(pBuffer, 0, BUFFER_SIZE);
        nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
        printf("amount read: %i\n", nReadAmount);
        printf("Response: \n%s",pBuffer);
        std::stringstream input;
        int i;
        //search for \r\n\r\n
        for(i = 0; i < nReadAmount; i++){
            printf("char: %c\n", pBuffer[i]);
            input << pBuffer[i];
        }
        std::string c_length = "Content-Length: ";
        std::size_t found = input.str().find(c_length);
        if (found!=std::string::npos){
            //get the amount to read
            std::stringstream toReadSS;
            int j = found + c_length.length();
            for(j = found; j < nReadAmount; j++){
                printf("char: %c\n", pBuffer[j]);
                toReadSS << pBuffer[j];
                if (pBuffer[j + 1] == '\r\n')
                    break;
            }
            toRead = atoi(toReadSS.str().c_str());
        }
    }
    if(close(hSocket) == SOCKET_ERROR)
    {
	printf("\nCould not close socket\n");
    }
    free(message);
}
