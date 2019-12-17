#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>


#define CLIENT_COUNT 10000

struct client {
    struct sockaddr_in address;
    socklen_t addr_len;
    int socket;
};


int main(void) {
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	struct protoent *protocol;
	socklen_t client_socklen;
	int server_socket;
	int client_socket;
	int enable = 1;
	ssize_t bytes_read;
	char *buffer;
	struct client clients[CLIENT_COUNT] = { 0 };
	size_t client_count = 0;

	printf("\nStarting server...");

	// open socket on host
	protocol = getprotobyname("tcp");
	server_socket = socket(AF_INET, SOCK_STREAM, protocol->p_proto);

	if (protocol == -1 || server_socket == -1) return -1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) return -1;

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(80);

	if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) return -1;
	if (listen(server_socket, 5) == -1) return -1;

	fcntl(server_socket, F_SETFL, fcntl(server_socket, F_GETFL, 0) | O_NONBLOCK);
	buffer = malloc(1028);

	printf("\nListening on port 80...\n");

	while (1) {
		if (client_count < sizeof clients / sizeof *clients) {
			client_socklen = sizeof(client_address);
			client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_socklen);

			if (client_socket != -1) {
				fcntl(client_socket, F_SETFL, fcntl(client_socket, F_GETFL, 0) | O_NONBLOCK);
				clients[client_count++] = (struct client) { .address = client_address, .addr_len = client_socklen, .socket = client_socket };

				printf("\nNew client connection: %d", client_count);                                                         
			}
		}

		for (size_t index = 0; index < client_count; index++) {
			ssize_t bytes_recvd = read(clients[index].socket, buffer, 1028);
			printf("\nReceived: %s", buffer);

			if (bytes_recvd == 0) {
				close(clients[index].socket);
				client_count--;
				memmove(clients + index, clients + index + 1, (client_count - index) * sizeof clients);
				continue;
			}
		}

		sleep(0);
	}
}
