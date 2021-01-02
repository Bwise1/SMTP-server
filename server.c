/* echo_server_thread.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** echo_server_thread.c                                                  ***/
/***                                                                       ***/
/*** An echo server using threads.                                         ***/
/***                                                                       ***/
/*** Compile : gcc echo_server_thread.c -o echo_server_thread -lpthread    ***/
/*****************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>

//iniparser Libraries
#include "iniparser.h"

//Variables
char *serverName;
char *domain;
	
/* Definations */
#define DEFAULT_BUFLEN 1024
#define PORT 2045

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }


void iniParserFunction(char * fileName);
int checkFile(char * , char *);

//global variables


/*--------------------------------------------------------------------*/
/*--- Child - echo server                                         ---*/
/*--------------------------------------------------------------------*/
void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
	//char welcome[DEFAULT_BUFLEN];
	char clientName[20], command[6], mailTo[20];
    int bytes_read;
    int client = *(int *)arg;
    
    sprintf(line, "220 %s <%s>\n",serverName,domain );
    
    send(client, line, strlen(line), 0);
	memset(line, '\0', strlen(line));
    do
    {	
        bytes_read = recv(client, line, sizeof(line), 0);
			
        if (bytes_read > 0) {
        	
        	
			strcpy(command, strtok(line, " "));
			strcpy(clientName, strtok(NULL, " "));
			printf("%s",clientName);
			clientName[strcspn(clientName, "\n")] = 0;
			sprintf(line, "250 Hello %s, Pleased to meet you\n", clientName);
			printf("%s",line);
			send(client, line, strlen(line), 0);
			memset(line, '\0', strlen(line));
			
			bytes_read = recv(client, line, sizeof(line), 0);
			if (bytes_read > 0) {
				strcpy(command, strtok(line, ":"));
				strcpy(mailTo, strtok(NULL, ":"));
				if(checkFile("ban_domain.cfg", "spammer.com"))
					printf("\n250 server ok");
			}
        	printf("\n%s, %s",command, mailTo);
        
        	/*if ( (bytes_read=send(client, line, bytes_read, 0)) < 0 ) {
                        printf("Send failed\n");
                        break;
                }*/
        } else if (bytes_read == 0 ) {
                printf("Connection closed by client\n");
                break;
        } else {
                printf("Connection has problem\n");
                break;
        }
    } while (bytes_read > 0);
    close(client);
    return arg;
}

/*--------------------------------------------------------------------*/
/*--- main - setup server and await connections (no need to clean  ---*/
/*--- up after terminated children.                                ---*/
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{   int sd,opt,optval;
    struct sockaddr_in addr;
    unsigned short port=0;
	
	
	iniParserFunction("server.ini");
	
	
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
                port=atoi(optarg);
                break;
        }
    }


    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( port > 0 )
                addr.sin_port = htons(port);
    else
                addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("System ready on port %d\n",ntohs(addr.sin_port));

    while (1)
    {
        int client, addr_size = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr*)&addr, &addr_size);
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if ( pthread_create(&child, NULL, Child, &client) != 0 )
            perror("Thread creation");
        else
            pthread_detach(child);  /* disassociate from parent */
    }
    return 0;
}

void iniParserFunction(char * fileName){
	
    dictionary  *ini ;
	int serverPort;
	
    ini = iniparser_load(fileName);
    if (ini==NULL) {
        fprintf(stderr, "unable to parse ini file: %s\n", fileName);
        
    }
    //iniparser_dump(ini, stderr);

    serverPort = iniparser_getint(ini, "server:ListenPort", -1);
    printf("%d this is \n", serverPort);
    
    serverName = iniparser_getstring(ini, "server:ServerName", NULL);
    domain = iniparser_getstring(ini, "server:DomainName", NULL);
    //strncpy(serverM, serverMsg,  strlen(serverMsg));
    printf("server msg:  %s\n Domain name: %s\n",serverName, domain);
        
}

int checkFile(char *file, char *str){
	 
	FILE *fptr;
	char * line = NULL;
	size_t len = 0;
    ssize_t read;
    int flag;
    
    fptr = fopen(file, "r");
    if (fptr == NULL) {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
        //exit(1);
    }

    // reads text until newline is encountered
    while ((read = getline(&line, &len, fptr)) != -1){
    	 //line[strcspn(line, "\n")] = 0;
    	 //printf("\n%s", str);
    	if (strstr(line,str )!=NULL) {
            flag = 1;
            break;
        }
        flag = 0;
        //printf("\nNot found");
	}
    
    fclose(fptr);
	return flag;	
}
