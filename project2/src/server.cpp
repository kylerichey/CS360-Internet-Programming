//============================================================================
// Name        : project2.cpp
// Author      : Kyle Richey
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5
#define PATH_MAX 255

using namespace std;

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;
    char inputPath[255];


    if(argc < 3)
      {
        printf("\nUsage: server <host-port> <dir-path>\n");
        return 0;
      }

	nHostPort=atoi(argv[1]);
	strcpy(inputPath, argv[2]);
	if(inputPath[0]=='/')
	{
		printf("\nRemoving leading / from %s",inputPath);
		char temp[255];
		int length = strlen(inputPath);
		for(int i=0;i<length;i++)
		{
			temp[i]=inputPath[i+1];
		}
		strcpy(inputPath,temp);

	}

	printf("\nThe port is %i\nThe Path is %s\n",nHostPort,inputPath);


	char *path=inputPath;
	//char *path = "test";

	struct stat path_stat;
	stat(path, &path_stat);
	bool isFile= S_ISREG(path_stat.st_mode);
	bool isDir = S_ISDIR(path_stat.st_mode);

	if(isDir)
	{

		//is a directory
		DIR *dirp;
		struct dirent *dp;
		dirp = opendir(inputPath);
		dp = readdir(dirp);
		printf("%s is a directory with the following files:\n",path);
		while ((dp = readdir(dirp)) != NULL)
			{
				printf("file %s\n", dp->d_name);
			}

		(void)closedir(dirp);
		}
	else if(isFile)
	{
		//is a file

		printf("THis is a file not a directory\n");
		return 0;
	}
	else
	{
		//isnt found
		printf("%s is not found",path);
	}



   // printf("\n%s", directoryPath);
    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d\n",nHostPort);

    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address))
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    while(1)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
              Address.sin_addr.s_addr,
              ntohs(Address.sin_port));
       // strcpy(pBuffer,MESSAGE);

        char xBuffer[1000];
        char buffer[1];
    /*	bool hasSeenNewLine = false;
    		int xBufferIndex = 0;
    		while (true) {

    			read(hSocket, buffer, 1);
    			if (buffer[0] == '\r') {
    				if (hasSeenNewLine) {
    					break;
    				}
    			}
    			if (buffer[0] == '\n') {
    				hasSeenNewLine = true;
    			} else {
    				hasSeenNewLine = false;
    			}

    			xBuffer[xBufferIndex++] = buffer[0];
    		}

       printf("Header Received:%s",xBuffer);*/



				int xBufferIndex = 0;
				while (true) {

					read(hSocket, buffer, 1);

					if (buffer[0] == '\n') {
						break;
					}

					xBuffer[xBufferIndex++] = buffer[0];
				}

		   printf("Get Received:%s",xBuffer);

		   if(xBuffer[0]!= 'G' || xBuffer[1]!= 'E' || xBuffer[2]!= 'T' || xBuffer[3]!= ' ')
		   {
			   printf("Error in get header");
			   return 0;
		   }

		  bool seenSpace =false;
		  int pathIndex=0;
		   xBufferIndex=4;
		  char path[255];
		  while(1)
		  {
			  if(xBuffer[xBufferIndex]==' ')
			  {
				  break;
			  }
			  path[pathIndex++]= xBuffer[xBufferIndex++];

		  }
		  printf("path:%s\n",path);
		  char abPath[255];
		  strcpy(abPath,inputPath);
		  strcpy(abPath,path);
		  printf("Absolute path:%s\n",abPath);

		  if(abPath[0] == ' ')
		  {
				//Give directory listing
		  }
		  else
		  {
			  //remove leading /

		  printf("\nRemoving leading / from %s",abPath);
				char temp[255];
				int length = strlen(abPath);
				for(int i=0;i<length;i++)
				{
					temp[i]=abPath[i+1];
				}
				strcpy(abPath,temp);


			 char *path2 = abPath;


			struct stat st2;
			stat(path2, &st2);
			int size = st2.st_size;
			 bool isFile= S_ISREG(st2.st_mode);
			 printf("\n%s is a file:%d", abPath, isFile);
			printf("%s is a file that has %i bytes\n",abPath,size);


			FILE *fp = fopen(abPath,"r");
			char *buf = (char *) malloc(size);
			fread(buf,size, 1, fp);
			printf("File info %s",buf);
			free(buf);
			fclose(fp);
		  }


       // printf("\nSending \"%s\" to client",pBuffer);
        /* number returned by read() and write() is the number of bytes
        ** read or written, with -1 being that an error occured
        ** write what we received back to the server */
      //  write(hSocket,pBuffer,strlen(pBuffer)+1);
        /* read from socket into buffer */
      //  memset(pBuffer,0,sizeof(pBuffer));
       // read(hSocket,pBuffer,BUFFER_SIZE);


      /*  if(strcmp(pBuffer,MESSAGE) == 0)
            printf("\nThe messages match");
        else
            printf("\nSomething was changed in the message"); */

       char outBuffer [2055];
      // strcpy(outBuffer,"<!DOCTYPE html> <html> <head> <title>Hi there</title> </head> </html>");
       write(hSocket,outBuffer,strlen(outBuffer)+1);

    printf("\nClosing the socket");
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
