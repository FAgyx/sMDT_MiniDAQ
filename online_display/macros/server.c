/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
//#ifdef __cplusplus
//extern "C" {
//#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "src/sockhelp.c"

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

int server(int portno)
{
	 int sockfd, newsockfd;
	 socklen_t clilen;
	 char server_ip_address[INET_ADDRSTRLEN],client_ip_address[INET_ADDRSTRLEN];
	 unsigned int buffer[4096];
	 struct sockaddr_in serv_addr, cli_addr;
	 int b,n,f,c;
	 int sockReadCount, bytes_recv, total_bytes_recv, total_elements_written;
	 FILE *incomingDataFile, *logFile;
	 size_t	uint_size, char_size;
  	 time_t localTime;
  	 struct tm *localDateTime;
     char myDateTime[30];

	 uint_size = sizeof(unsigned int);	// 4
	 char_size = sizeof(char); 			// 1

	 /*if (argc < 2) {
		 fprintf(stderr,"ERROR, no port provided\n");
		 exit(1);
	 }*/
	 sockfd = socket(AF_INET, SOCK_STREAM, 0);
	 if (sockfd < 0) 
		error("ERROR opening socket");
	 bzero((char *) &serv_addr, sizeof(serv_addr));
	 serv_addr.sin_family = AF_INET;
	 serv_addr.sin_addr.s_addr = INADDR_ANY;
	 //serv_addr.sin_addr.s_addr = inet_addr("141.211.96.35");
	 serv_addr.sin_port = htons(portno);

	 inet_ntop(AF_INET, &(serv_addr.sin_addr), server_ip_address, INET_ADDRSTRLEN);
	 printf("Waiting for client to connect to IP: %s Port: %u\n", server_ip_address, portno);
	 
	 b = ::bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
	 if (b < 0) error("ERROR on binding");

	 listen(sockfd,5);
	 clilen = sizeof(cli_addr);
	 newsockfd = accept(sockfd, 
				 (struct sockaddr *) &cli_addr, 
				 &clilen);
	 if (newsockfd < 0) 
		  error("ERROR on accept");

	 inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip_address, INET_ADDRSTRLEN);
	 printf("Connected to client IP:%s\n", client_ip_address);

	 incomingDataFile = NULL;
	 incomingDataFile = fopen("received_data.dat","wb");
     if (incomingDataFile != NULL) printf("Incoming data file is opened.\n");
     else printf("Unable to open incoming data file.\n");

     /*logFile = NULL;
     logFile = fopen("serv_socket.log","w");
     if (logFile == NULL) printf("Unable to open log file.\n");*/

     /*localTime = time(NULL);
  	 localDateTime = localtime(&localTime);
  	 strftime(myDateTime, 30, "%Y%m%d", localDateTime);
  	 sscanf(myDateTime,"%x", &DAQStartDate);
  	 strftime(myDateTime, 30, "%H%M%S", localDateTime);*/


     total_bytes_recv = 0;
     total_elements_written = 0;
     bzero(buffer,sizeof(buffer));
     bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
     total_bytes_recv += bytes_recv;
     sockReadCount = 1;
     printf("\nReceiving data...\n");
	 while (bytes_recv > 0) {
	 	 printf("sizeof(buffer) = %lu\n",sizeof(buffer) );
	 	 printf("buffer[4085] = %x\n",buffer[4085]);
	 	 printf("buffer[4095] = %x\n",buffer[4095]);
	 	 f = fwrite(buffer, sizeof(unsigned int), bytes_recv, incomingDataFile);
	 	 if (f != bytes_recv)
	 	 {
	 	 	printf("Error writing to file: f = %i, bytes_recv = %i\n",f,bytes_recv);
	 	 }
	 	 total_elements_written += f;
		 if (f < 0) error("ERROR writing to file");

		 //fprintf(logFile, "%u - socket received %i bytes.\n",sockReadCount,bytes_recv);

	 	 bytes_recv = sock_read(newsockfd, (char *) buffer, sizeof(buffer));
	 	 total_bytes_recv += bytes_recv;
	 	 sockReadCount++;
	 }
	 //f = fwrite(buffer, sizeof(unsigned int), sizeof(buffer), incomingDataFile);
	 //if (f < 0) error("ERROR writing to file");
	 //The last sock_read returns -1 since the client closed socket so final write doesn't happen

	 fclose(incomingDataFile);
	 close(newsockfd);
	 close(sockfd);
	 printf("Files and sockets closed.\n");
	 printf("Socket was read %u times.\n", sockReadCount);
	 printf("Socket received %u bytes of data.\n", total_bytes_recv);
	 printf("%i elements were writtent to the file\n",total_elements_written);

	 return 0; 
}

//#ifdef __cplusplus
//}
//#endif