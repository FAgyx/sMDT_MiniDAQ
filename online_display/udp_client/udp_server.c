// Server side implementation of UDP client-server model 
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
    char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    int bytes_recv, total_bytes_recv, bytes_write, total_bytes_write, sockReadCount;
    unsigned int data_buffer[8192];
    int datafile;  

    bzero(data_buffer,sizeof(data_buffer));
    bytes_recv = 0;
    total_bytes_recv = 0; 
    total_bytes_write = 0;
    datafile = 0;
    datafile = open("write.dat",O_WRONLY|O_CREAT);
    if(datafile>0){
        printf("file opened for write\n");
    }



    int len;
    len = sizeof(cliaddr);  //len is value/resuslt 
    // printf("bytes_recv = %d\n",bytes_recv);

    bytes_recv = recvfrom(sockfd, data_buffer, sizeof(data_buffer),  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
    printf("bytes_recv = %d\n",bytes_recv);

    // bytes_recv = read(sockfd, (char *) data_buffer, sizeof(data_buffer));
    // printf("bytes_recv=%d\n",bytes_recv);

    sockReadCount = 1;

    while (bytes_recv > 0) {
        total_bytes_recv += bytes_recv;
        bytes_write = write(datafile, (char *) data_buffer,bytes_recv);
        printf("bytes_write=%d\n",bytes_write);

        bytes_recv = recvfrom(sockfd, data_buffer, sizeof(data_buffer),  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        printf("bytes_recv = %d\n",bytes_recv);

        total_bytes_write += bytes_write;
        if(bytes_recv<sizeof(data_buffer)){
            bytes_write = write(datafile, (char *) data_buffer,bytes_recv);
            total_bytes_write += bytes_write;
            total_bytes_recv += bytes_recv;
            break;
        }
    }
    close(datafile);
    printf("Total_bytes_recv=%d, total_bytes_write=%d\n",total_bytes_recv,total_bytes_write);






      
    // int len, n; 
  
    // len = sizeof(cliaddr);  //len is value/resuslt 
  
    // n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
    //             MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
    //             &len); 
    // buffer[n] = '\0'; 
    // printf("Client : %s\n", buffer); 
    // sendto(sockfd, (const char *)hello, strlen(hello),  
    //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
    //         len); 
    // printf("Hello message sent.\n");  
      
    return 0; 
} 