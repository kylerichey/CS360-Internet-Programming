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
#include <sys/signal.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100

#define QUEUE_SIZE          5

using namespace std;

bool doesFileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

const char *get_filename_ext(const char *filename) {
	const char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

void signalHandler (int status)
{
	printf("\nReceived signal %d\n",status);
	exit(0);
}

int main(int argc, char* argv[]) {

	int hSocket, hServerSocket; /* handle to socket */
	//struct hostent* pHostInfo; /* holds info about a machine */
	struct sockaddr_in Address; /* Internet socket address stuct */
	int nAddressSize = sizeof(struct sockaddr_in);
	int nHostPort;
	char inputPath[255];

	if (argc < 3) {
		printf("\nUsage: server <host-port> <dir-path>\n");
		return 0;
	}



	// First set up the signal handler
	struct sigaction sigold, signew;
	signew.sa_handler=signalHandler;
	sigemptyset(&signew.sa_mask);
	sigaddset(&signew.sa_mask,SIGINT);
	sigaddset(&signew.sa_mask,SIGSEGV);
	sigaddset(&signew.sa_mask,SIGFPE);
	signew.sa_flags = SA_RESTART;
	sigaction(SIGINT,&signew,&sigold);
	sigaction(SIGSEGV,&signew,&sigold);
	sigaction(SIGFPE,&signew,&sigold);

	nHostPort = atoi(argv[1]);
	strcpy(inputPath, argv[2]);

	char *path = inputPath;
	struct stat path_stat;
	stat(path, &path_stat);
	bool isDir = S_ISDIR(path_stat.st_mode);

	if (isDir) {
		printf("\nThe port is %i\nThe Path is %s\nThe path is valid\n",
				nHostPort, inputPath);

	} else {
		//isnt found
		printf("The path is not a directory - %s", path);
		return 0;
	}

	printf("\nStarting server");

	printf("\nMaking socket");
	/* make a socket */
	hServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	int optval = 1;
	setsockopt(hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	if (hServerSocket == SOCKET_ERROR) {
		printf("\nCould not make a socket\n");
		return 0;
	}

	/* fill address struct */
	Address.sin_addr.s_addr = INADDR_ANY;
	Address.sin_port = htons(nHostPort);
	Address.sin_family = AF_INET;

	printf("\nBinding to port %d\n", nHostPort);

	/* bind to a port */
	if (bind(hServerSocket, (struct sockaddr*) &Address,
			sizeof(Address)) == SOCKET_ERROR) {
		printf("\nCould not connect to host\n");
		return 0;
	}
	/*  get port number */
	getsockname(hServerSocket, (struct sockaddr *) &Address,
			(socklen_t *) &nAddressSize);


	printf("\nMaking a listen queue of %d elements", QUEUE_SIZE);
	/* establish listen queue */
	if (listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR) {
		printf("\nCould not listen\n");
		return 0;
	}

	while (1) {

		printf("\nWaiting for a connection\n");
		/* get the connected socket */
		hSocket = accept(hServerSocket, (struct sockaddr*) &Address,
				(socklen_t *) &nAddressSize);

		printf("\nGot a connection from %X (%d)\n", Address.sin_addr.s_addr,
				ntohs(Address.sin_port));

		char *buffer = (char *) malloc(1000);
		char *path = (char *) malloc(1000);
		char *abPath = (char *) malloc(1000);

		read(hSocket, buffer, 1000);

		if (buffer[0] != 'G') {
			printf("Invalid Get, Dropping Connection\n");
		} else {

			char * pch;
			pch = strtok(buffer, " ");
			pch = strtok(NULL, " ");
			//printf("%s\n", pch);
			strcpy(path, pch);
			free(buffer);
			strcpy(abPath, inputPath);
			strcat(abPath, path);
			free(path);

			if (strstr(abPath, "favicon") != NULL) {
				printf("Favicon request, Sending 404\n");

				char *header = (char *) malloc(1000);
				strcpy(header, "HTTP/1.1 404 NOT FOUND\r\n\r\n");
				write(hSocket, header, strlen(header));
				free(abPath);
				free(header);

			} else {
				printf("Absolute path:%s\n", abPath);

				DIR *dirp;
				struct dirent *dp;
				dirp = opendir(abPath);
				if (dirp != NULL) {

					//check if index.html exists

					char *indexPath = (char *) malloc(1000);
					strcpy(indexPath, abPath);
					strcat(indexPath, "index.html");
					printf("%s\n",indexPath);
					if (doesFileExists(indexPath)) {

						printf("Sending Index.html\n");
						//printf("File exists, sending to client\n");

						struct stat st3;
						stat(indexPath, &st3);
						int size = st3.st_size;
						char *body = (char *) malloc(size+1);
						char *header = (char *) malloc(1000);


						FILE *fp = fopen(indexPath, "r");
						fread(body, size, 1, fp);
						fclose(fp);

						strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n");
						char *b = (char *) malloc(50);
						sprintf(b,
								"Content-Length: %i\r\nConnection: keep-alive\r\n\r\n",
								strlen(body));
						strcat(header, b);
						free(b);

						write(hSocket, header, strlen(header));
						write(hSocket, body, strlen(body));

						free(body);
						free(header);
						free(abPath);

						free(indexPath);

					} else {
						free(indexPath);
						//is a dir
						dp = readdir(dirp);
						int size = 0;
						while ((dp = readdir(dirp)) != NULL) {
							size = size + (strlen(dp->d_name)) + 23;
						}
						size += 500 + strlen(path);
						(void) closedir(dirp);
						dirp = opendir(abPath);
						dp = readdir(dirp);

						char *body = (char *) malloc(size);
						char *header = (char *) malloc(1000);

						char *c = (char *) malloc(100);
						sprintf(c,
								"<html> <head> <title>Directory Listing for %s </title> </head> <body> Name <hr />",
								path);
						strcat(body, c);
						free(c);

						while ((dp = readdir(dirp)) != NULL) {
							//	printf("file %s\n", dp->d_name);
							char *d = (char *) malloc(100);
							//<a href="oral.html"> oral.html</a><br />
							sprintf(d, "<a href=\"%s\"> %s</a><br />",
									dp->d_name, dp->d_name);
							//printf("%i\n",strlen(d));
							strcat(body, d);
							free(d);
						}

						char *e = (char *) malloc(50);
						sprintf(e, " </body> </html> ");
						strcat(body, e);
						free(e);

						strcpy(header,
								"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n");
						char *b = (char *) malloc(50);
						sprintf(b, "Content-Length: %i\r\n\r\n", strlen(body));
						strcat(header, b);
						free(b);

						//printf("%i %i\n",strlen(body),size);
						write(hSocket, header, strlen(header));
						write(hSocket, body, strlen(body));

						free(body);
						free(header);
						free(abPath);
					}
				} else if (doesFileExists(abPath)) {
					printf("File exists, sending to client\n");

					struct stat st2;
					stat(abPath, &st2);
					int size = st2.st_size;
					char *body = (char *) malloc(size);
					char *header = (char *) malloc(1000);

					//printf("\n%s is a file:%d", abPath, isFile);
					//printf("%s is a file that has %i bytes\n", abPath, size);
					FILE *fp = fopen(abPath, "r");
					strcpy(header, "HTTP/1.1 200 OK\r\n");

					const char *extension = get_filename_ext(abPath);

					if (strcmp(extension, "html") == 0) {
						//printf("is html\n");
						strcat(header, "Content-Type: text/html\r\n");
					} else if (strcmp(extension, "txt") == 0) {
						//printf("is txt\n");
						strcat(header, "Content-Type: text/plain\r\n");
					} else if (strcmp(extension, "gif") == 0) {
						//printf("is gif\n");
						strcat(header, "Content-Type: image/gif\r\n");
					} else if (strcmp(extension, "jpg") == 0) {
						//printf("is jpg\n");
						strcat(header, "Content-Type: image/jpg\r\n");
					} else {
						//plain?
						strcat(header, "Content-Type: text/html\r\n");
					}

					char *b = (char *) malloc(50);
					sprintf(b,
							"Content-Length: %i\r\nConnection: keep-alive\r\n\r\n",
							size);
					strcat(header, b);
					free(b);
					fread(body, size, 1, fp);
					write(hSocket, header, strlen(header));
					write(hSocket, body, size);

					free(body);
					free(header);
					free(abPath);
					fclose(fp);

				} else {
					//printf("Not a file or a driectory\n");
					//send 404
					char *header = (char *) malloc(1000);
					char *body = (char *) malloc(1000);
					strcpy(header,
							"HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n");
					char *b = (char *) malloc(50);
					sprintf(b,
							"Content-Length: %i\r\nConnection: keep-alive\r\n\r\n",
							46);
					strcat(header, b);
					free(b);

					strcpy(body,
							"<html><body>404 - File Not Found</body></html>");
					//printf("%i",strlen(body));
					write(hSocket, header, strlen(header));
					write(hSocket, body, strlen(body));
					free(abPath);
					free(header);
					free(body);

				}
				(void) closedir(dirp);
			}

			linger lin;
			unsigned int y = sizeof(lin);
			lin.l_onoff = 1;
			lin.l_linger = 10;
			setsockopt(hSocket, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));
		}
		printf("\nClosing the socket");
		/* close socket */
		if (close(hSocket) == SOCKET_ERROR) {
			printf("\nCould not close socket\n");
			return 0;
		}

	} //while1
}
