#include "../library.h"

int server_fd, new_socket;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);
char buffer[1024] = {0};
char *hello = "C: Hello from C";
int epoll_fd;

int main(int argc, char const *argv[]) {

	int running = 1, event_count, i;
	int bytes_read=-1;
	char read_buffer[READ_SIZE + 1];
	struct epoll_event event, events[MAX_EVENTS];
	
	epoll_fd = epoll_create1(0);

	if (epoll_fd == -1) {
		fprintf(stderr, "Failed to create epoll file descriptor\n");
		return 1;
	}

	createSocket(&server_fd, &address, &opt, PORTC);

	add_to_epoll(&event, &epoll_fd, &server_fd);

	while (running) {
		event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
		// printf("%d ready events\n", event_count);
		for (i = 0; i < event_count; i++) {
			
			/* Add new block when some nodes wants to listen on same socket */

			if (events [i].data.fd == server_fd) {			
				new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
				
				read_in_buffer(new_socket, read_buffer, &bytes_read);

				/* Add this block for every new event */

				if (read_buffer[0] == 'B') {
					printf("Read '%s'\n", read_buffer);
				
					/* Decide which function to call to send message (Same socket, different socket) */
					sendOnSameSocket(&new_socket, hello);
				}	
			}
		}
	}

	if (close(epoll_fd)<0) {
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		return 1;
	}
	
	return 0;
}
