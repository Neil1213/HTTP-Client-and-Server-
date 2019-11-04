#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#define CONNMAX 1000
#define BYTES 1024

char *ROOT;
int listenfd, clients[CONNMAX];
void error(char *);
void start(char *);
void res(int);

void res(int n)
{
	char mes[99999], *req[3], dts[BYTES], path[99999];
	int rcvd, fd, read_bytes;

	memset((void *)mes, (int)'\0', 99999);

	rcvd = recv(clients[n], mes, 99999, 0);

	if (rcvd < 0)
	{
		fprintf(stderr, ("RECV error\n"));
	}
	else
	{
		if (rcvd == 0)
		{
			fprintf(stderr, "UNEXPECTED DISCONNECTION\n");
		}

		else
		{
			req[0] = strtok(mes, " \t\n");

			if (strncmp(req[0], "GET\0", 4) == 0)
			{
				req[1] = strtok(NULL, " \t");
				req[2] = strtok(NULL, " \t\n");

				if (strncmp(req[2], "HTTP/1.0", 8) != 0 && strncmp(req[2], "HTTP/1.1", 8) != 0)
				{
					write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
				}
				else
				{
					if (strncmp(req[1], "/\0", 2) == 0)
					{
						req[1] = "/index.html";
					}

					strcpy(path, ROOT);
					strcpy(&path[strlen(ROOT)], req[1]);

					if ((fd = open(path, O_RDONLY)) != -1)
					{
						send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
						while ((read_bytes = read(fd, dts, BYTES)) > 0)
						{
							write(clients[n], dts, read_bytes);
						}
					}
					else
					{
						strcpy(path, ROOT);
						strcpy(&path[strlen(ROOT)], "/response404.txt");
						fd = open(path, O_RDONLY);
						send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);

						while ((read_bytes = read(fd, dts, BYTES)) > 0)
						{
							write(clients[n], dts, read_bytes);
						}
					}
				}
			}
			else
			{
				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], "/response501.txt");
				fd = open(path, O_RDONLY);
				send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);

				while ((read_bytes = read(fd, dts, BYTES)) > 0)
				{
					write(clients[n], dts, read_bytes);
				}
			}
		}
	}

	shutdown(clients[n], SHUT_RDWR);
	close(clients[n]);
	clients[n] = -1;
}

void start(char *port)
{
	struct addrinfo hints, *res, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &res) != 0)
	{
		perror("GETADDRINFO error");
		exit(1);
	}

	for (p = res; p != NULL; p = p->ai_next)
	{
		listenfd = socket(p->ai_family, p->ai_socktype, 0);

		if (listenfd == -1)
		{
			continue;
		}

		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
		{
			break;
		}
	}

	if (p == NULL)
	{
		perror("SOCKET or BIND error");
		exit(1);
	}

	freeaddrinfo(res);

	if (listen(listenfd, 1000000) != 0)
	{
		perror("LISTEN error");
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;

	char PORT[6];
	ROOT = getenv("PWD");
	strcpy(PORT, "10000");

	int slot = 0;

	if (argc == 3)
	{
		ROOT = malloc(strlen(argv[2]));
		strcpy(ROOT, argv[2]);
		strcpy(PORT, argv[1]);
	}
	else
	{
		printf("ERROR: Invalid arguments.\n");
		exit(1);
	}

	printf("Starting server at port number %s%s%s sharing %s%s%s\n", "\033[92m", PORT, "\033[0m", "\033[92m", ROOT, "\033[0m");

	int i;
	for (i = 0; i < CONNMAX; i++)
	{
		clients[i] = -1;
	}
	start(PORT);

	while (1)
	{
		addrlen = sizeof(clientaddr);
		clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

		if (clients[slot] < 0)
			error("ACCEPT error");
		else
		{
			if (fork() == 0)
			{
				res(slot);
				exit(0);
			}
		}

		while (clients[slot] != -1)
		{
			slot = (slot + 1) % CONNMAX;
		}
	}

	return 0;
}