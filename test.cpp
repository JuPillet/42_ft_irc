#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <string>


int main()
{
	int sockfd, new_sock;
	int opt = 1;
	struct sockaddr_in address;
	char buf[1024];
	char msg[] = "Message received";

	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_family = AF_INET;
	address.sin_port = htons(4242);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
	bind(sockfd, (struct sockaddr*)&address, sizeof(address));
	listen(sockfd, 1);
	while (1)
	{
		int addrlen = sizeof(address);
		new_sock = accept(sockfd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
		read(new_sock, buf, 1024);
		std::cout << buf << std::endl;
		send(new_sock, msg, strlen(msg), 0);
	}
}