#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include "enc_dec/include/MessageAB.h"

/* Change the ports when new node is added */
#define PORTB 8082
#define PORTC 8083
#define PORTA 8081
#define MAX_EVENTS 50
#define READ_SIZE 500

void encode(MessageAB_t *ab, char** enc_msg)
{
    asn_enc_rval_t ec;
    char temp[100000] = {};
    ec = aper_encode_to_buffer(&asn_DEF_MessageAB, 0, ab, temp, 100000);
    if (ec.encoded == -1)
    {
        fprintf(stderr, "Could not encode MessageAB(at %s)\n",
                ec.failed_type ? ec.failed_type->name : "unknown");
        exit(1);
    }
    else
    {
        // fprintf(stderr, "Encoded Message is: %s\n", temp);
        fprintf(stderr, "Encoded %ld bits\n", ec.encoded);
    }
    *enc_msg=temp;
}

void decode(char *temp, MessageAB_t **dec_msg)
{
    MessageAB_t *dec = 0;
    asn_dec_rval_t decRet;
    char errBuf[10000] = {};
    size_t errlen = 10000;
    decRet = aper_decode_complete(0, &asn_DEF_MessageAB, (void **)&dec, (char *)temp, 100000);

    if (asn_check_constraints(&asn_DEF_MessageAB, dec, errBuf, &errlen) < 0)
        printf("check constraints failed: %s", errBuf);

    if (decRet.code == RC_OK)
    {
        printf("Decode Successfull\n");
        *dec_msg = dec;
    }
    else
    {
        printf("Decode failed, Err %s", decRet.code == RC_FAIL ? "RC_FAIL" : "RC_WMORE");
        *dec_msg = NULL;
    }
}

void createSocket(int *server_fd, struct sockaddr_in *address, int *opt, int port) {
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, opt, sizeof(*opt));

	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = htons(port);

	bind(*server_fd, (struct sockaddr *)address, sizeof(*address));
	listen(*server_fd, 3);
}

void add_to_epoll (struct epoll_event *event, int *epoll_fd, int *server_fd) {
	event->events = EPOLLIN | EPOLLOUT;
	event->data.fd = *server_fd;

	if (epoll_ctl(*epoll_fd, EPOLL_CTL_ADD, *server_fd, event) < 0) {
		fprintf(stderr, "Failed to add file descriptor to epoll\n");
		close(*epoll_fd);
		return;
	}
}

void read_in_buffer (int new_socket, char read_buffer[], int *bytes_read) {
	while (*bytes_read < 0) {
		*bytes_read = read(new_socket, read_buffer, READ_SIZE);
	}
	read_buffer[*bytes_read] = '\0';
	*bytes_read = -1;
}


void sendToClient(int port, char msg[]) {
    int sock = -1, valread;
	struct sockaddr_in serv_addr;

    printf("Enter in client\n");

	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0) {
		printf("Error in creating socket\n");
	}
		
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))< 0) {
		printf("Error in making connection\n");
    }
	if ((send(sock , msg , strlen(msg) , 0 )) < 0) {
		printf("Error in sending the message\n");
	}
}

void sendOnSameSocket(int *new_socket, char msg[]) {
	if ((send(*new_socket , msg , strlen(msg) , 0 )) < 0) {
		printf("Error in sending message\n");
	}
}

void receiveFromClientOnSamePort(int port, char msg[], int *new_socket2, int *epoll_fd) {
	int valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	struct epoll_event event;
	
	*new_socket2 = socket(AF_INET, SOCK_STREAM, 0);
	
	if (*new_socket2 < 0) {
		printf("Error creating socket\n");
	}
		
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

	if (connect(*new_socket2, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error in making connection\n");
		return;
	}

	add_to_epoll(&event, epoll_fd, new_socket2);

	if ((send(*new_socket2, msg, strlen(msg), 0)) < 0) {
		printf("Error in Sending The Message\n");
		return;
	}
}