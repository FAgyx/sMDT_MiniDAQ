
// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <fcntl.h>

  
 
  
// Driver code 
int udp_client(int port_no, const char *read_file_name) { 
    int sockfd; 

    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port_no); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 


    int datafile;    
    unsigned int data_buffer[4096];
    bzero(data_buffer,sizeof(data_buffer));
    int bytes_read, total_bytes_read, bytes_send, total_bytes_send;

    total_bytes_read=0;
    total_bytes_send=0;

    datafile = open(read_file_name,O_RDONLY);
    bytes_read = read(datafile, (char *) data_buffer, sizeof(data_buffer));
    // printf("bytes_read=%d\n",bytes_read);
    while(bytes_read>0){
        // printf("bytes_read=%d\n",bytes_read);
        total_bytes_read += bytes_read;
        // bytes_send = write(sockfd, (char *) data_buffer, bytes_read);
        bytes_send = sendto(sockfd, data_buffer, bytes_read, 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        total_bytes_send += bytes_send;
        bytes_read = read(datafile, (char *) data_buffer, sizeof(data_buffer));
    }
    close(datafile);
    // printf("Total_bytes_read=%d, total_bytes_send=%d\n",total_bytes_read,total_bytes_send);
 
    close(sockfd); 
    return 0; 
} 
