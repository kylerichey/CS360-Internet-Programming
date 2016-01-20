#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <vector>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         555
#define HOST_NAME_SIZE      255
#define MAX_MSG_SZ     		1024
#define MAXMSG 				1024

using namespace std;

// Determine if the character is whitespace
bool isWhitespace(char c) {
	switch (c) {
	case '\r':
	case '\n':
	case ' ':
	case '\0':
		return true;
	default:
		return false;
	}
}

// Strip off whitespace characters from the end of the line
void chomp(char *line) {
	int len = strlen(line);
	while (isWhitespace(line[len])) {
		line[len--] = '\0';
	}
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds) {
	char tline[MAX_MSG_SZ];
	char *line;

	int messagesize = 0;
	int amtread = 0;
	while ((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ) {
		if (amtread >= 0)
			messagesize += amtread;
		else {
			perror("Socket Error is:");
			fprintf(stderr,
					"Read Failed on file descriptor %d messagesize = %d\n", fds,
					messagesize);
			exit(2);
		}
		//fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
		if (tline[messagesize - 1] == '\n')
			break;
	}
	tline[messagesize] = '\0';
	chomp(tline);
	line = (char *) malloc((strlen(tline) + 1) * sizeof(char));
	strcpy(line, tline);
	//fprintf(stderr, "GetLine: [%s]\n", line);
	return line;
}

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str) {
	int i;
	char *s;

	s = str;
	for (i = 0; s[i] != ':'; i++) {
		if (s[i] >= 'a' && s[i] <= 'z')
			s[i] = 'A' + (s[i] - 'a');

		if (s[i] == '-')
			s[i] = '_';
	}

}

// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, const char *prefix) {
	char *result = (char *) malloc(strlen(str) + strlen(prefix));
	char* value = strchr(str, ':') + 1;
	UpcaseAndReplaceDashWithUnderline(str);
	*(strchr(str, ':')) = '\0';
	sprintf(result, "%s%s=%s", prefix, str, value);
	return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(vector<char *> &headerLines, char * pBuffer,
		bool envformat) {
	// Read the headers, look for specific ones that may change our responseCode
	char *line;
	char *tline;

	tline = strtok(pBuffer, "\n");
	//printf ("%s\n",pch);
	// tline = GetLine(skt);
	while ((tline != NULL) && (strlen(tline) != 0)) {
		if (strstr(tline, "Content-Length") || strstr(tline, "Content-Type")) {
			if (envformat)
				line = FormatHeader(tline, "");
			else
				line = strdup(tline);
		} else {
			if (envformat)
				line = FormatHeader(tline, "HTTP_");
			else {
				line = (char *) malloc((strlen(tline) + 10) * sizeof(char));
				sprintf(line, "HTTP_%s", tline);
			}
		}
		//fprintf(stderr, "Header --> [%s]\n", line);

		headerLines.push_back(line);
		//free(tline);
		//tline = GetLine(skt);
		tline = strtok(NULL, "\n");

	}
	free(tline);
}

int main(int argc, char **argv) {

	int hSocket; /* handle to socket */
	struct hostent* pHostInfo; /* holds info about a machine */
	struct sockaddr_in Address; /* Internet socket address stuct */
	long nHostAddress;
	char pBuffer[1];
	char xBuffer[BUFFER_SIZE];
	char strHostName[HOST_NAME_SIZE];
	char hostPort[HOST_NAME_SIZE];
	int nHostPort = 0;
	char strURLName[HOST_NAME_SIZE];

	int arg;
	int argOffset = 1;
	bool debug = false;
	bool repeat = false;
	char *cvalue = NULL;
	opterr = 0;

	while ((arg = getopt(argc, argv, "c:d")) != -1) {
		switch (arg) {
		case 'c':
			repeat = true;
			argOffset++;
			argOffset++;
			cvalue = optarg;
			break;
		case '?':
			if (optopt == 'c')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		case 'd':
			debug = true;
			argOffset++;
			break;
		default:
			cout << "USAGE: download [-args] [host] [port] [URL] " << endl;
			exit(0);
		}
	}

	if (argc < 4) {
		cout << "USAGE: download [-args] [host] [port] [URL] " << endl;
		exit(0);
	}


	string hostname = argv[argOffset];
	string port = argv[argOffset + 1];
	string url = argv[argOffset + 2];

	string path = hostname + ":" + port + url;
	int repeatCounter = 1;
	if (repeat) {
		repeatCounter = atoi(cvalue);

		printf("Repeat Mode Enabled\n");
	}
	while (repeatCounter > 0) {

		if (repeat) {
			printf("%i repeats to go\n", repeatCounter);

		}

		if (debug) {
			cout << endl << "Argument data:" << endl;
			cout << "Hostname: " << hostname << endl;
			cout << "Port: " << port << endl;
			cout << "URL: " << url << endl;
			cout << "Path: " << path << endl;
			cout << "Debug Mode: " << boolalpha << debug << endl;
			cout << "Repeat Mode: " << boolalpha << repeat << " " << cvalue
					<< " repeats" << endl;
			cout << endl;
		}

		//printf("\nMaking a strHostName");
		strcpy(strHostName, argv[argOffset]);
		//printf("\nMaking a nHostPort");
		strcpy(hostPort, argv[argOffset + 1]);

		//	printf("%s\n", hostPort);
		int hostPortLength = strlen(hostPort);
		//	printf("%i\n", hostPortLength);
		for (int i = 0; i < hostPortLength; i++) {
			if (!isdigit(hostPort[i])) {
				printf("\nInvalid Port\n");
				return 0;
			}
		}

		nHostPort = atoi(hostPort);
		if (nHostPort < 1 || nHostPort > 65000) {
			printf("\nInvalid Port\n");
			return 0;
		}

		strcpy(strURLName, argv[argOffset + 2]);

		if (debug) {
			printf("\nMaking a socket");
		}
		/* make a socket */
		hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (hSocket == SOCKET_ERROR) {
			printf("\nCould not make a socket\n");
			return 0;
		}

		/* get IP address from name */
		pHostInfo = gethostbyname(strHostName);

		if (!pHostInfo) {
			printf("Could not resolve host name\n");
			return 0;
		}

		/* copy address into long */
		memcpy(&nHostAddress, pHostInfo->h_addr,pHostInfo->h_length);

		/* fill address struct */
		Address.sin_addr.s_addr = nHostAddress;
		Address.sin_port = htons(nHostPort);
		Address.sin_family = AF_INET;

		if (debug) {
			printf("\nConnecting to %s (%X) on port %d", strHostName,
					(unsigned int) nHostAddress, nHostPort);
		}

		/* connect to host */
		if (connect(hSocket, (struct sockaddr*) &Address,
				sizeof(Address)) == SOCKET_ERROR) {
			printf("\nCould not connect to host\n");
			return 0;
		}

		//create a message

		char * message = (char *) malloc(MAXMSG);

		sprintf(message, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", strURLName,
				strHostName);
		if (debug) {
			printf("\nMessage :\n%s\n", message);
		}
		write(hSocket, message, strlen(message));
		memset(pBuffer, 0, 1);

		/* read from socket into buffer
		 ** number returned by read() and write() is the number of bytes
		 ** read or written, with -1 being that an error occured */

		bool hasSeenNewLine = false;
		int xBufferIndex = 0;
		while (true) {

			read(hSocket, pBuffer, 1);
			if (pBuffer[0] == '\r') {
				if (hasSeenNewLine) {
					break;
				}
			}
			if (pBuffer[0] == '\n') {
				hasSeenNewLine = true;
			} else {
				hasSeenNewLine = false;
			}

			xBuffer[xBufferIndex++] = pBuffer[0];
		}

		vector<char *> headerLines;

		char contentLength[MAX_MSG_SZ];

		//printf ("Splitting string \"%s\" into tokens:\n",pBuffer);

		GetHeaderLines(headerLines, xBuffer, false);

		if (debug) {
			printf("Response Header\n");
		}
		// Now print them out
		for (int i = 0; i < headerLines.size(); i++) {
			if (debug) {
				printf("[%d] %s\n", i, headerLines[i]);
			}
			if (strstr(headerLines[i], "Content-Length")) {
				sscanf(headerLines[i], "Content-Length: %s", contentLength);
			}
		}

		if (debug) {
			printf("Content Length is %s\n\n", contentLength);
		}

		read(hSocket, pBuffer, 1);
		int length = atoi(contentLength);
		char * buffer = (char *) malloc(length);
		//line = (char *) malloc((strlen(tline) + 10) * sizeof(char));
		read(hSocket, buffer, length);

		if (!repeat || debug) {
			printf("HTML Body\n");
			for (int i = 0; i < length; i++) {
				printf("%c", buffer[i]);
			}
		}
		// Now read and print the rest of the file

		/* close socket */
		if (close(hSocket) == SOCKET_ERROR) {
			printf("\nCould not close socket\n");
			return 0;
		}
		free(message);
		free(buffer);
		repeatCounter--;
	}
}
