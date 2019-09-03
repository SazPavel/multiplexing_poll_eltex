#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

int sock_tcp, sock_udp, sock_temp;

void SigintHandler(int sig)
{
    close(sock_tcp);
    close(sock_udp);
    printf("\nserver close\n");
    exit(0);
}

int main()
{
    struct sigaction sigint;    
    char buf[16];
    int bytes_read, result;
    struct sockaddr_in addr_tcp, addr_udp;
    struct pollfd sd[2];
    
    sigint.sa_handler = SigintHandler;
    sigint.sa_flags = 0;
    sigemptyset(&sigint.sa_mask);
    sigaddset(&sigint.sa_mask, SIGINT);
    sigaction(SIGINT, &sigint, 0);
    
    sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_tcp < 0 || sock_udp < 0)
    {
        perror("socket");
        exit(-1);
    }
    
    addr_tcp.sin_family = AF_INET;
    addr_tcp.sin_port = htons(3935);
    addr_tcp.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(sock_tcp, (struct sockaddr*)&addr_tcp, sizeof(addr_tcp)) < 0)
    {
        perror("bind_tcp");
        exit(-1);
    }
    
    addr_udp.sin_family = AF_INET;
    addr_udp.sin_port = htons(3936);
    addr_udp.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(sock_udp, (struct sockaddr*)&addr_udp, sizeof(addr_udp)) < 0)
    {
        perror("bind_udp");
        exit(-1);
    }
    
    if(listen(sock_tcp, 1) < 0)
    {
        perror("listen");
        exit(-1);
    }
    
    sd[1].fd = sock_udp;
    sd[0].fd = sock_tcp;
    sd[0].events = POLLIN;
    sd[1].events = POLLIN;    
    
    while(1)
    {
        result = poll(sd, 2, -1);
        if(result > 0)
        {
	        if(sd[0].revents & POLLIN)
	        {
	            sock_temp = accept(sock_tcp, NULL, NULL);
	            if(sock_temp < 0)
	            {
	                perror("accept");
	                close(sock_tcp);
	                close(sock_udp);
	                exit(-1);
	            }
	            bytes_read = recv(sock_temp, buf, 16, 0);
	            printf("TCP %s\n", buf);
	            close(sock_temp);
	        }
	        if(sd[1].revents & POLLIN)
	        {
	            recvfrom(sock_udp, buf, 16, 0, NULL, NULL);
	            printf("UDP %s\n", buf);
	        }
	}
    }
    exit(0);
}
