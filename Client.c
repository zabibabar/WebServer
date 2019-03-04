//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#include <sys/types.h>    // Needed for sockets stuff
#include <netinet/in.h>   // Needed for sockets stuff
#include <sys/socket.h>   // Needed for sockets stuff
#include <arpa/inet.h>    // Needed for sockets stuff
#include <fcntl.h>        // Needed for sockets stuff
#include <netdb.h>        // Needed for sockets stuff
#include <errno.h>
#include <stdbool.h>
//----- Defines ---------------------------------------------------------------
#define  IP_ADDR    	"127.0.0.1"  // IP address of server (*** HARDWIRED ***)

//----- Global Variables-------------------------------------------------------
int ports[3];
bool knock_send(int port);

//===== Main program ==========================================================
int main()
{
	knock_send(8079);
	int i;
	for (i = 0; i < 3; i++ ){
		ports[i] = ports[i]/43691;
		//printf("%d\n", ports[i]);
	}

	sleep(1);
	if (knock_send(ports[0]))
	{
		sleep(0.1);
		if (knock_send(ports[1]))
		{
			sleep(0.1);		
			if (knock_send(ports[2]))
			{	
				printf("Web server is Open!\n");
				//knock_send(8080);
			}
		}	
	}		
	// Return zero and terminate
	return(0);
}

bool knock_send(int port)
{
	int                  client_s;        		// Client socket descriptor
	struct sockaddr_in   server_addr;     		// Server Internet address
	//char                 out_buf[BUFFER_SIZE];  // Output buffer for data
	//char                 in_buf[BUFFER_SIZE];   // Input buffer for data
	int                  retcode;        		// Return code

		// >>> Step #1 <<<
	// Create a client socket
	//   - AF_INET is Address Family Internet and SOCK_STREAM is streams
	client_s = socket(AF_INET, SOCK_STREAM, 0);
	int optval = 1;
    setsockopt(client_s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (client_s < 0)
	{
		printf("*** ERROR - socket() failed \n");
		printf("Error Code = %i, Error = %s \n", errno, strerror(errno));
		return false;
	}

	// >>> Step #2 <<<
	// Fill-in the server's address information and do a connect with the
	// listening server using the client socket - the connect() will block.
	server_addr.sin_family = AF_INET;                 // Address family to use
	server_addr.sin_port = htons(port);           // Port num to use
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDR); // IP address to use
	printf("Attempting to connect at port %d \n", port);
	retcode = connect(client_s, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (retcode < 0)
	{
		printf("*** ERROR - connect() failed \n");
		printf("Error Code = %i, Error = %s \n", errno, strerror(errno));
		return false;
	}
	printf("Successfully connect to port %d \n", port);

	// >>> Step #3 <<<
	// Receive from the server using the client socket
	retcode = recv(client_s, ports, sizeof(ports), 0);
	if (retcode < 0)
	{
		printf("*** ERROR - recv() failed \n");
		return false;
	}

	// >>> Step #5 <<<
	// Close the client socket	
	//retcode = ;
	if (close(client_s) < 0)
	{
		printf("*** ERROR - close() failed \n");
		return false;
	}	
	return true;
}