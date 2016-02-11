#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <cmath>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define HOST_NAME_SIZE      255
//#define NSOCKETS 10
#define NSTD 3

float standard_deviation(float data[], int n)
{
    float mean=0.0, sum_deviation=0.0;
    int i;
    for(i=0; i<n;++i)
    {
        mean+=data[i];
    }
    mean=mean/n;
    for(i=0; i<n;++i)
    sum_deviation+=(data[i]-mean)*(data[i]-mean);
    return sqrt(sum_deviation/n);
}

int main(int argc, char* argv[]) {
	int nSockets=1;
	struct hostent* pHostInfo; /* holds info about a machine */
	struct sockaddr_in Address; /* Internet socket address stuct */
	long nHostAddress;
	char pBuffer[BUFFER_SIZE];

	char strHostName[HOST_NAME_SIZE];
	char strPathName[HOST_NAME_SIZE];
	int nHostPort;

	printf("Number of Args: %i\n",argc);
	if (argc < 5) {
			printf("\nUsage: webtest hostname port path -d <count> \n");
			return 0;
		}

	int arg;
	int argOffset = 1;
	bool debug = false;
	opterr = 0;

	while ((arg = getopt(argc, argv, "d")) != -1) {
		switch (arg) {
		case '?':
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		case 'd':
			debug = true;
			argOffset++;
			break;
		default:
			printf("\nUsage: webtest hostname port path -d <count> \n");
			exit(0);
		}
	}


	strcpy(strHostName, argv[argOffset]);
	nHostPort = atoi(argv[argOffset+1]);
	strcpy(strPathName, argv[argOffset +2]);
	nSockets =  atoi(argv[argOffset+3]);

	if(nSockets<1)
	{
		printf("Please enter a positive non-zero number of sockets\n");
		return 0;
	}

	printf("Host name: %s\n",strHostName);
	printf("Port: %i\n",nHostPort);
	printf("Path: %s\n", strPathName);
	printf("Debug: %s\n", debug?"true":"false");
	printf("Socket Count: %i\n\n",nSockets);

	int hSocket[nSockets]; /* handle to socket */
	struct timeval oldtime[nSockets + NSTD];


	/* make a socket */
	int i;
	for (i = 0; i < nSockets; i++) {
		hSocket[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (hSocket[i] == SOCKET_ERROR) {
			printf("\nCould not make a socket\n");
			return 0;
		}
	}

	printf("Made %i sockets\n\n", nSockets);

	/* get IP address from name */
	pHostInfo = gethostbyname(strHostName);
	/* copy address into long */
	memcpy(&nHostAddress, pHostInfo->h_addr,pHostInfo->h_length);

	/* fill address struct */
	Address.sin_addr.s_addr = nHostAddress;
	Address.sin_port = htons(nHostPort);
	Address.sin_family = AF_INET;



	int epollFD = epoll_create(1);
	// Send the requests and set up the epoll data
	for (i = 0; i < nSockets; i++) {
		/* connect to host */
		if (connect(hSocket[i], (struct sockaddr*) &Address,
				sizeof(Address)) == SOCKET_ERROR) {
			printf("\nCould not connect to host\n");
			return 0;
		}
		//char request[] = "GET /test1.html HTTP/1.0\r\n\r\n";
		char request[] = "GET ";
		strcat(request, strPathName);
		strcat(request," HTTP/1.0\r\n\r\n");
		//printf("Request: %s",request);

		write(hSocket[i], request, strlen(request));
		// Keep track of the time when we sent the request
		gettimeofday(&oldtime[hSocket[i]], NULL);
		// Tell epoll that we want to know when this socket has data
		struct epoll_event event;
		event.data.fd = hSocket[i];
		event.events = EPOLLIN;
		int ret = epoll_ctl(epollFD, EPOLL_CTL_ADD, hSocket[i], &event);
		if (ret)
			perror("epoll_ctl");
	}

	double totalRequestTime = 0;
	float standardDeviationData[nSockets];

	for (i = 0; i < nSockets; i++) {
		struct epoll_event event;
		int rval = epoll_wait(epollFD, &event, 1, -1);
		if (rval < 0)
			perror("epoll_wait");
		rval = read(event.data.fd, pBuffer, BUFFER_SIZE);
		struct timeval newtime;
		// Get the current time and subtract the starting time for this request.
		gettimeofday(&newtime, NULL);
		double usec = (newtime.tv_sec - oldtime[event.data.fd].tv_sec)
				* (double) 1000000
				+ (newtime.tv_usec - oldtime[event.data.fd].tv_usec);
		//std::cout << "Time "<<usec/1000000<<std::endl;
		double socketTime = usec / 1000000;
		totalRequestTime +=socketTime;
		standardDeviationData[i]=socketTime;
		if(debug)
		{
			printf("Socket %d took %f seconds\n",event.data.fd, socketTime );
		}
		//printf("got %d from %d\n", rval, event.data.fd);
		// Take this one out of epoll
		epoll_ctl(epollFD, EPOLL_CTL_DEL, event.data.fd, &event);

	}

	// Now close the sockets
	for (i = 0; i < nSockets; i++) {
		//printf("\nClosing socket\n");
		/* close socket */
		if (close(hSocket[i]) == SOCKET_ERROR) {
			printf("\nCould not close socket\n");
			return 0;
		}
	}
	printf("All Sockets closed\n");

	double averageResponseTime = totalRequestTime/nSockets;
	float standardDeviation = standard_deviation(standardDeviationData,nSockets);

	printf("Total Request time for %i sockets is %f\n", nSockets, totalRequestTime);
	printf("The average response time is %f seconds\n",averageResponseTime);
	printf("The standard deviation is %f\n", standardDeviation);
	return 0;
}
