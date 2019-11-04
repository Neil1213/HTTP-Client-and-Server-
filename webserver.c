#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#define BUF_SIZE (10 * 1024)
#define PATHNAME_SIZE 1024
#define SIZE_PER_READ (32 * 1024)
/* half of TCP max packet size */

char *webroot;

const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1>The requested resource could not be found but may be available again in the future.<div style=\"color: #eeeeee; font-size: 8pt;\">Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></html>";
const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</body></html>";

const char *HTTP_200_STRING = "OK";
const char *HTTP_404_STRING = "Not Found";
const char *HTTP_501_STRING = "Not Implemented";

enum content_type {
	ct_html,  	/* 0 */
	ct_css,		/* 1 */
	ct_jpg,		/* 2 */
	ct_png,		/* 3 */
	ct_gif,		/* 4 */
	ct_plain,	/* 5 */
	ct_js,		/* 6 */
	ct_woff,	/* 7 */
	ct_none,
};

struct content_type_info {
	char *extension;
	char *header_line;
};

struct content_type_info content_type_list[] = {
	{"html", "Content-Type: text/html\r\n"},
	{"css", "Content-Type: text/css\r\n"},
	{"jpg", "Content-Type: image/jpeg\r\n"},
	{"png", "Content-Type: image/png\r\n"},
	{"gif", "Content-Type: image/gif\r\n"},
	{"txt", "Content-Type: text/plain\r\n"},
	{"js", "Content-Type: text/javascript\r\n"},
	{"woff", "Content-Type: font/woff\r\n"}
};

struct thread_parameter{
	int sock;
	pthread_t thread;
};

/* return the length of a request saved in req_header. Part of the next request may
 * also be received. The function updates next_req_offset and next_req_len to mark the
 * part of next request, which will be moved to the beginning of req_header when the
 * function is called next time before it receives more contents from the client.
 * 
 * return -1 if recv() returns -1 or \r\n\r\n cannot be found within a certain length,
 * such that the worker thread (caller) should close the connection and terminate.
 */

int read_request(int sock, char *req_header, int *next_req_offset, int *next_req_len)
{
}

/*
 * return 0, if it is a GET request and is formated well.
 * return -1, if it is NOT a GET request, or HTTP version is NOT 1.1, if URL cannot be successfully extracted
 * if it is a GET request, the function also extracts URL and looks at connection header field.
 * It saves 1 into *keep_alive if there is a "connection: keep_alive" found; it saves 0 otherwise.
 * pathname of the file to be sent to client is saved as a NULL-ended string in URL.
 * If pathname is not found, URL[0] is 0.
 */
int parse_request(char *req, int len, char *URL, int *keep_alive) {
}


int send_501_resp(int sock, char *resp_header, int keep_alive){
}


int send_404_resp(int sock, char *resp_header, int keep_alive){
}

/* look up the content_type_list, and return a Content-Type header line based on
 * the file extension contained in pathname
 */
char *get_content_type(char *pathname)
{
}

int send_200_resp(int sock, char *resp_header, int keep_alive, char *pathname, int fd)
{
	char *file_buf[SIZE_PER_READ];

	/* generate a header. You need to get the content type based on URL to generate 
         * the Content-Type line, get the size of the page/object in URL (pathname) to
         * generate the Content-Length line, generate the Connection line based on
         * keep_alive parameter.
         */

	/* send the header and extra \r\n */

	/* In a loop, keep reading data from the page/object requested and send the data to
	 * the client. SIZE_PER_READ is recommended batch size of data in each read: 32KB
         */
}

void *worker_thread(void * param)
{
	struct thread_parameter *thread_info = (struct thread_parameter *)param;
	int sock = thread_info->sock;
	char req_header[BUF_SIZE], resp_header[BUF_SIZE];
	char URL[PATHNAME_SIZE], pathname_obj[PATHNAME_SIZE];
	int req_header_len = 0, resp_header_len = 0, next_req_offset = 0, next_req_len = 0;
	int keep_alive = 0, URL_len = 0, method = 0;
	int resp_sent = 0;
	int error404;
	int fd_obj;
	struct timeval tv;

	pthread_detach(thread_info->thread); /* Detaching avoids the main thread to call pthread_join */

	tv.tv_sec = 5;  /* 5 Secs Timeout */

	/* With the time-out, the thread can wake up from recv() if the client becomes idle and
         * stops sending new request. The server should have a mechanism to release the resource
         * dedicated to a client when it becomes idle.
         */
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

	while (1) {
		/* get a request with read_request() */
		
		/* parse the request with parse_request() */

		/* based on the results from parse_request(), decide whether it should send
                 * a HTTP 501 response, and do so by calling send_501_resp() if it should
                 */
	

		/* if it should not send a HTTP 501 response, based on the results from 
		 * parse_request(), decide whether it should send a HTTP 404 response, 
		 * and do so by calling send_404_resp() if it should
                 */

		/* send a normal response.*/
	}

	/* without shutdown, since the main thread still alive and have the socket open, the
         * client will continue to send new requests to the connection, which no one is serving.
         */
	shutdown(sock, SHUT_RDWR);
	free(param);
}

int main(int argc, char **argv)
{
	int sock, conn, port_number, len, i;
	struct sockaddr_in server_addr, client_addr; 
	struct thread_parameter *thread_param;

	if(argc != 3 | sscanf(argv[1], "%d", &port_number) != 1) {
		fprintf(stderr, "%s port_number webroot\n", argv[0]);
		exit(1);
	}

	webroot = argv[2];

	/* create socket, bind it to an address, and put it into listening mode.
         * print an error message (perror) if bind fails
	 */

	while(1){
		conn = accept(...);
		if (conn == -1) { 
			perror("accept");
			continue;
		}
		/* We use malloc, since we don't know how many worker threads are needed and cannot
		 * use an array.
		 */
		thread_param = (struct thread_parameter *)malloc(sizeof(struct thread_parameter));
		thread_param->sock = conn;
		pthread_create(&thread_param->thread, NULL, worker_thread, thread_param);
	}
	return 0;
}
