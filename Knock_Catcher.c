#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

//----- Function Protocol -----------------------------------------------------
bool knock_recv(int port);

//===== Main program ==========================================================
int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++)
    {
        if(!knock_recv(atoi(argv[i]))){
            return -1;
        }

    }

    return 0;
}   

bool knock_recv(int port)
{
	int                  welcome_s;       // Welcome socket descriptor
	struct sockaddr_in   server_addr;     // Server Internet address
	int                  connect_s;       // Connection socket descriptor
	struct sockaddr_in   client_addr;     // Client Internet address
	struct in_addr       client_ip_addr;  // Client IP address
	int                  addr_len;        // Internet address length
	int                  retcode;         // Return code
	pthread_t            thread_id;  
	
	// Create a welcome socket
    //   - AF_INET is Address Family Internet and SOCK_STREAM is streams
    welcome_s = socket(AF_INET, SOCK_STREAM, 0);
    if (welcome_s < 0)
    {
        printf("*** ERROR - socket() failed \n");
        printf("Error Code = %i, Error = %s \n", errno, strerror(errno));
        return false;
    }
	
	int optval = 1;
	setsockopt(welcome_s, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    // Fill-in server (my) address information and bind the welcome socket
    server_addr.sin_family = AF_INET;                 // Address family to use
    server_addr.sin_port = htons(port);           // Port number to use
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on any IP address
    retcode = bind(welcome_s, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (retcode < 0)
    {
        printf("*** ERROR - bind() failed \n");
        printf("Error Code = %i, Error = %s \n", errno, strerror(errno));
        return false;
    }
       
    // Listen on welcome socket for a connection
    listen(welcome_s, 1);

    // Accept a connection.  The accept() will block and then return with
    // connect_s assigned and client_addr filled-in.
    printf("Waiting for accept() to complete at port %d \n", port);
    addr_len = sizeof(client_addr);
    connect_s = accept(welcome_s, (struct sockaddr *)&client_addr, &addr_len);
    if (connect_s < 0)
    {
        printf("*** ERROR - accept() failed \n");
        return false;
    }

    // Copy the four-byte client IP address into an IP address structure
    memcpy(&client_ip_addr, &client_addr.sin_addr.s_addr, 4);

    printf("Accept completed (IP address of client = %s  port = %d) \n",
    inet_ntoa(client_ip_addr), ntohs(client_addr.sin_port));	

	//Close the welcome and connect sockets
    retcode = close(welcome_s);
    if (retcode < 0)
    {
        printf("*** ERROR - close() failed \n");
        return false;
    }

    retcode = close(connect_s);
    if (retcode < 0)
    {
        printf("*** ERROR - close() failed \n");
        return false;
    }

	return true;
}
