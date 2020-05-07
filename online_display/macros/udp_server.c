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
  

  
// Driver code 
int udp_server(int port_no, const char* inputfile_name) { 
    int sockfd; 
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
    servaddr.sin_port = htons(port_no); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    int bytes_recv, total_bytes_recv, bytes_write, total_bytes_write, sockReadCount;
    unsigned int data_buffer[4096];
 

    bzero(data_buffer,sizeof(data_buffer));

    // FILE *datafile;    
    // datafile = fopen(inputfile_name,"w");
    // if(datafile==NULL){
    //     printf("Can not open file!\n");
    //     return 0;
    // }

    int datafile=0;
    datafile = open(inputfile_name,O_RDWR|O_CREAT);
    if(datafile<=0){
        printf("Can not open file!\n");
        return 0;
    }

    socklen_t len;
    len = sizeof(cliaddr);  //len is value/resuslt 


    bytes_recv = 0;
    total_bytes_recv = 0; 
    total_bytes_write = 0;
    //recvfrom until next complete UDP socket series 
    while (1) {
        bytes_recv = recvfrom(sockfd, data_buffer, sizeof(data_buffer), 
            MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
        // printf("bytes_recv = %d\n",bytes_recv);
        if(bytes_recv<sizeof(data_buffer)) break;
    }


    while (1) {
        bytes_recv = recvfrom(sockfd, data_buffer, sizeof(data_buffer), 
            MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
        total_bytes_recv += bytes_recv;
        // bytes_write = fwrite((char *)data_buffer, 1, bytes_recv, datafile);
        bytes_write = write(datafile, (char *) data_buffer,bytes_recv);
        total_bytes_write += bytes_write;
        // printf("bytes_recv = %d\n",bytes_recv);
        // printf("bytes_write= %d\n",bytes_write);      
        
        
        if(bytes_recv<sizeof(data_buffer))break;        
    }
    // fclose(datafile);
    close(datafile);
    printf("Total_bytes_recv=%d, total_bytes_write=%d\n",total_bytes_recv,total_bytes_write);

 
      
    return 1; 
} 