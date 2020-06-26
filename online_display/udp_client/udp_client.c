
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

  
#define PORT     8080 
#define MAXLINE 1024 
  
// Driver code 
int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "Hello from client"; 
    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 


    int datafile;    
    unsigned int data_buffer[8192];
    bzero(data_buffer,sizeof(data_buffer));
    int bytes_read, total_bytes_read, bytes_send, total_bytes_send;

    total_bytes_read=0;
    total_bytes_send=0;

    datafile = open("read.dat",O_RDONLY);
    bytes_read = read(datafile, (char *) data_buffer, sizeof(data_buffer));
    // printf("bytes_read=%d\n",bytes_read);
    while(bytes_read>0){
        // printf("bytes_read=%d\n",bytes_read);
        total_bytes_read += bytes_read;
        // bytes_send = write(sockfd, (char *) data_buffer, bytes_read);
        bytes_send = sendto(sockfd, data_buffer, bytes_read, 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr));
        printf("bytes_send = %d\n",bytes_send);
        total_bytes_send += bytes_send;
        bytes_read = read(datafile, (char *) data_buffer, sizeof(data_buffer));
    }
    close(datafile);
    printf("Total_bytes_read=%d, total_bytes_send=%d\n",total_bytes_read,total_bytes_send);


      
    // int n, len; 
      
    // sendto(sockfd, (const char *)hello, strlen(hello), 
    //     MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
    //         sizeof(servaddr)); 
    // printf("Hello message sent.\n"); 
          
    // n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
    //             MSG_WAITALL, (struct sockaddr *) &servaddr, 
    //             &len); 
    // buffer[n] = '\0'; 
    // printf("Server : %s\n", buffer); 
  
    close(sockfd); 
    return 0; 
} 
