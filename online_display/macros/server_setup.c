/*
This function setups a TCPIP server  binding sockfd to server_ip_int:portno.
newsockfd is the socket file descriptor for read out.
*/




#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


void server_setup(int* sockfd, int* newsockfd, in_addr_t server_ip_int, short portno){
	socklen_t clilen;
	char server_ip_address[INET_ADDRSTRLEN],client_ip_address[INET_ADDRSTRLEN];
	struct sockaddr_in serv_addr, cli_addr;

	// Creating socket file descriptor 
	if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
	perror("socket failed"); 
	exit(EXIT_FAILURE); 
	}
	//Forcefully attaching socket to the port number from main() argument
    //error for local IP
    int opt = 1;
    if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = server_ip_int;
	serv_addr.sin_port = htons(portno);
	inet_ntop(AF_INET, &(serv_addr.sin_addr), server_ip_address, INET_ADDRSTRLEN);
	printf("Waiting for client to connect to IP: %s Port: %u\n", server_ip_address, portno);

	// Forcefully attaching socket  
    if (bind(*sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(*sockfd, 5) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    clilen = sizeof(cli_addr);
    if ((*newsockfd = accept(*sockfd, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }
    time_t local_time;
    local_time = time(0);
	inet_ntop(AF_INET, &(cli_addr.sin_addr), client_ip_address, INET_ADDRSTRLEN);
	printf("Connected to client IP:%s\n at %s\n", client_ip_address, ctime(&local_time));
    return;	
}