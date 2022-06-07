#include "../library.h"

int server_fd1, new_socket1, new_socket2 = -1;
struct sockaddr_in address1;
int opt1 = 1;
int addrlen = sizeof(address1);
char *hello1 = "B: Hello from B";
int epoll_fd;

int main(int argc, char const *argv[]) {

	int running = 1, event_count;
	int bytes_read = -1;
	char read_buffer1[READ_SIZE + 1];
	
	struct epoll_event event, events[MAX_EVENTS];

	epoll_fd = epoll_create1(0);

	if (epoll_fd == -1) {
		fprintf(stderr, "Failed to create epoll file descriptor\n");
		return 1;
	}

	createSocket(&server_fd1, &address1, &opt1, PORTB);

	add_to_epoll(&event, &epoll_fd, &server_fd1);

	while (running) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
		// printf("%d ready events\n", event_count);
		
		for (int i = 0; i < event_count; i++) {
			if ((events[i].events & EPOLLIN) == EPOLLIN) {

				/* Only add this block for new node add	*/

				if (events[i].data.fd == server_fd1) {
					new_socket1 = accept(server_fd1, (struct sockaddr *)&address1, (socklen_t *)&addrlen);
					read_in_buffer(new_socket1, read_buffer1, &bytes_read);
					MessageAB_t *ab;
					decode(read_buffer1,&ab);
					if (strcmp(ab->msgType.buf,"uplink") == 0) {
						printf("Received Message Type is: '%s'\n", ab->msgType.buf);	
						printf("Received Message is: '%s'\n", ab->msg.buf);	
						printf("Unique ID is: '%ld'\n", ab->unqID);	

						/* Decide which function to call to send message (Same socket, different socket) */
						
						receiveFromClientOnSamePort(PORTC, hello1, &new_socket2, &epoll_fd);
					}
					
				}
				else if (events[i].data.fd == new_socket2) {
					read_in_buffer(new_socket2, read_buffer1, &bytes_read);
					
					if (read_buffer1[0] == 'C') {
						printf("Read '%s'\n", read_buffer1);

						/* Decide which function to call to send message (Same socket, different socket) */
						MessageAB_t *btoa;
						btoa = calloc(1, sizeof(MessageAB_t)); 
						btoa->msgType.buf = "downlink";                 
						btoa->msgType.size = strlen(btoa->msgType.buf); 
						btoa->unqID = 99999;                             
						btoa->msg.buf = "Message from b to a";            
						btoa->msg.size = strlen(btoa->msg.buf);         
						char *enc_msg;
						encode(btoa, &enc_msg);
						sendToClient(PORTA, enc_msg);
					}
					
					close(new_socket2);
				}
				
			}
		}
	}

	if (close(epoll_fd) < 0) {
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		return 1;
	}

	return 0;
}

