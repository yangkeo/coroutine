#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1)
	{
		perror("socket");
		exit(1);
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9000);
	addr.sin_addr.s_addr = inet_addr("172.17.254.41");
	
	int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
	if(ret == -1)
	{
		perror("connect");
		exit(1);
	}

	char buf[1024] = {0};
	while(fgets(buf, 1024, stdin) != NULL)
	{
		send(fd, buf, strlen(buf), 0);
		memset(buf, 0, sizeof(buf));
		int ret = recv(fd, buf, 1024, 0);
		if(ret <= 0)
			break;

		printf("=> %s", buf);
	}
	return 0;
}