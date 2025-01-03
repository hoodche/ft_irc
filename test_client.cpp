#include <sys/socket.h>
#include <cstdio>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

int main()
{
	int					sockfd;
	char				input[4096 + 1];
	struct sockaddr_in	servaddr;

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
		printf("socket error");
        exit(1);
    }

	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(1234);
	if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0)
    {
		printf("inet_pton error");
        exit(1);
    }
	if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
		printf("connect error");
        exit(1);
    }
	while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL)
            break;//client finished with  CTRL + D
		if (write(sockfd, input, strlen(input)) < 0)
		{
			printf("write error");
			exit(1);
		}
	}
	exit(0);
}