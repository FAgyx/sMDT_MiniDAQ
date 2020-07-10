
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include "sockhelp.h"
#include <fcntl.h>
#include <time.h>
#define PORT 12346





   
int main(int argc, char const *argv[]) 
{ 
    int sock = 0, valread; 
    time_t sys_time;
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    char *server_address_str;
    server_address_str = "127.0.0.1";


    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    sys_time = time(0);
    printf("Connected to %s at %s\n", server_address_str, ctime(&sys_time));


    /*
    send(sock , hello , strlen(hello) , 0 ); 
    printf("Hello message sent\n"); 
    valread = read( sock , buffer, 1024); 
    printf("%s\n",buffer ); 
    */
    int datafile;    
    unsigned int buffer[4096];
    bzero(buffer,sizeof(buffer));
    int bytes_read, total_bytes_read, bytes_send, total_bytes_send;

    total_bytes_read=0;
    total_bytes_send=0;

    datafile = open("fake_data.dat",O_RDONLY);
    bytes_read = read(datafile, (char *) buffer, sizeof(buffer));
    // printf("bytes_read=%d\n",bytes_read);
    while(bytes_read>0){
    	// printf("bytes_read=%d\n",bytes_read);
    	total_bytes_read += bytes_read;
    	bytes_send = sock_write(sock, (char *) buffer, bytes_read);
    	total_bytes_send += bytes_send;
    	bytes_read = read(datafile, (char *) buffer, sizeof(buffer));
    }
    close(datafile);
    printf("Total_bytes_read=%d, total_bytes_send=%d\n",total_bytes_read,total_bytes_send);
    return 0; 


    
} 

