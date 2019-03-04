//Group members: Zabeeh Ullah Babar and Sayde King
//----- Include files ---------------------------------------------------------
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
#include <time.h>
#include <math.h>       
#include <sys/select.h>   
//----- Defines ---------------------------------------------------------------
#define BUFFER_SIZE 1024
#define MAX_NUM_CONNECTIONS 10
#define PORT_NUM 8079

//----- Global Variables-------------------------------------------------------
pid_t pid;
void kill_child(int sig);
bool knock_recv(int port);
int* randomPortsGenerator();
int* portHasher(int*);
void* ServerHandler (void* arg);
char *str[3];
int secret[10] = {43691, 43801, 43853, 43867, 43889, 43891, 43913, 43933, 43943, 43951};

//===== Main program ==========================================================
int main()
{
	pthread_t   tid[10];          /* process id for user threads */
    pthread_attr_t	attr[1];
 
    /* Required to schedule thread independently.*/
    pthread_attr_init(&attr[0]);
    pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);
    /* end to schedule thread independently */

	// Receive the first knock and send the hashedPorts to (for loop) each user	
	int i;
	for (i = 0; i < MAX_NUM_CONNECTIONS; i++)
	{
		printf("Waiting for user #%d\n", i+1);
		if (knock_recv(PORT_NUM)) 
		{
			pthread_create(&tid[i], &attr[0], ServerHandler, NULL);
			pthread_join(tid[i], NULL);
		}	
	}
}

void* ServerHandler (void* arg)
{

	signal(SIGALRM,(void (*)(int))kill_child);
	pid = fork();

	if (pid < 0)
	{
		perror("fork failed");
		exit(1);
	}

	else if (pid == 0)
	{
		//Copy the str array into a variable
		char* args[] = {"./Knock_Catcher", str[0], str[1], str[2], NULL};
		execv (args[0], args);	
	}
	
	alarm(10);
	int waitstatus;
    wait(&waitstatus);

    if (WIFEXITED(waitstatus))
    {
    	int knockReturn = WEXITSTATUS(waitstatus);
    	if (knockReturn == 0)
    	{
    		printf("Knocks successful.\n");

			//Load Weblite here
			signal(SIGALRM,(void (*)(int))kill_child);
			pid = fork();

			if (pid < 0){
				perror("fork failed");
				exit(1);
			}
			else if (pid == 0)
			{
				char* args[] = {"./weblite", NULL};
				execv (args[0], args);	
			}
			else 
			{
				alarm(10);
				int waitserver;
    			wait(&waitserver);
    			if (WIFSIGNALED(waitserver))
				{
					printf("Web Server Closed.\n");
					return (void *) 0;
				}
    		}

    	}
    	else if (knockReturn == -1)
    		printf("Knocks failed.\n");
    }
	else if (WIFSIGNALED(waitstatus))
	{
		printf("Time ran out.\n");
	}
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
	struct timeval       timeout;       // Struct for time interval for select()
  	fd_set               recvsds;       // Recieve descriptor for select()
	
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

   	// Send to the client using the connect socket
   	int *ports;
   	ports = randomPortsGenerator();
   	
   	int *hashedPorts;
   	hashedPorts = portHasher(ports);

	retcode = send(connect_s, hashedPorts, 12, 0);
	if (retcode < 0)
	{
		printf("*** ERROR - send() failed \n");
		return false;
	}	

	int i;
	for (i = 0; i < 3; i++)
 		printf("%02X", hashedPorts[i]);
	printf("\n"); 

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

int* portHasher(int* ports)
{
	static int hashedPorts[3];
	int i;
	for (i = 0; i < 3; i++)
	{
		hashedPorts[i] = ports[i]*secret[0];
		//printf("%d\n", hashedPorts[i]);
		//https://primes.utm.edu/glossary/page.php?sort=WagstaffPrime
	}

	return hashedPorts;
}

void kill_child(int sig)
{
    kill(pid,SIGKILL);
}

int* randomPortsGenerator() 
{ 	
	static int ports[3];
    srand (time(NULL));
	int i; 
    for (i = 0; i < 3; i++) { 
        int num = (rand() % (49151 - 1024 + 1)) + 1024; 
    	ports[i] = num;
    	int length = snprintf( NULL, 0, "%d", ports[i]);
		str[i] = malloc( length + 1 );
		snprintf( str[i], length + 1, "%d", ports[i]);
    }

    return ports;
} 

