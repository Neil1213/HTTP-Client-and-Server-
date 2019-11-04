#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
const char *HTTP_200_STRING = "OK";
const char *HTTP_404_STRING = "Not Found"; 
const char *HTTP_501_STRING = "Not Implemented"; 

const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body ><h1 > 404 Not Found</h1>The server cannot find requested resource. </html>"; 
const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body ><h1 color = >501 Not Implemented</h1>The request method is not supported by the server and cannot be handled. The only method that server is required to support is GET.</body></html>"; 
#define BACKLOG 10
#define MAX_BUFFER 2000
#define MAXLINE 100

#define SERVER_ROOT "./web/"
//new connection handler
void * reqHand(void *);
int urlParser(char *url, char *filename);
void main(int argc, char* argv[]) {

	/* variables for connection management */
  int parentfd;          /* parent socket */
  int childfd;           /* child socket */
  int portno;            /* port to listen on */
  int clientlen;         /* byte size of client's address */
  struct hostent *hostp; /* client host info */
  char *hostaddrp;       /* dotted decimal host addr string */
  int optval;            /* flag value for setsockopt */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
pthread_t thread_id;

	/* check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);
  puts("server started...");
  /* open socket descriptor */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
   {
   	puts("ERROR opening socket");
   }

   puts("socket created...");
		
  /* allows us to restart server immediately */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /* bind port to socket */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    {
    	puts("ERROR on binding");
    }

	puts("Binding done...");	

	/* get us ready to accept connection requests */
  if (listen(parentfd, BACKLOG) < 0)
   {
   	puts("ERROR on listen");\
   }

   puts("waiting for incomming connection...");
   while( (childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen)) )
    {
 
        if( pthread_create( &thread_id , NULL ,  reqHand , (void*) &childfd) < 0)
        {
           perror("could not create thread");
           return 1;
        }
         
        //pthread_join( thread_id , NULL);

        puts("waiting for incomming connection...");
	}
	
	close(parentfd);
	puts("server has terminated...");

	return 0;
}

void * reqHand(void * mSock)
{
	int sock = *(int*)mSock;
	int read_size;
	char buffer[MAX_BUFFER];
	char method[MAXLINE], url[MAXLINE], version[MAXLINE],filename[MAXLINE];
	char fileType[MAXLINE],connection[MAXLINE];
	char conntype[MAXLINE];
	char *tmp;
     int i=0;
    //Receive a message from client
    while((read_size = recv(sock , buffer , MAX_BUFFER , 0)) > 0 )
    {
        //end of string marker
		buffer[read_size] = '\0';
		//puts(buffer);
		sscanf(buffer, "%s %s %s", method, url, version);
		if (strcasecmp(method, "GET")) {
	    reponseError(sock,"501", HTTP_501_STRING,HTTP_501_CONTENT, version,conntype);
	    continue;
		}

		//verify content type static or dynmaic
		if(urlParser(url,filename)==0)
		{
			reponseError(sock,"501",HTTP_501_STRING,HTTP_501_CONTENT, version,conntype);
	    continue;
		}

		//verify connection field
		tmp=strstr(buffer,"Connection:");
		if(tmp)
		{
			sscanf(tmp,"Connection: %s",conntype);
			//printf("conn %s\n",conntype );
			if(strcasecmp(conntype,"Keep-Alive")==0)
			{
				strcpy(connection,"keep-alive");
			}
			else
			{
				strcpy(connection,"close");
			}
		}
		else
		{
			strcpy(connection,"close");
		}
		//get  file
		if(strstr(filename, "..") != NULL)
		{
				reponseError(sock,"404", HTTP_404_STRING,HTTP_404_CONTENT, version,conntype);
		}
		
		struct stat st; 
 		int filesize;
    	if(stat(filename, &st)<0)
    	{
    		reponseError(sock ,"404",HTTP_404_STRING,HTTP_404_CONTENT, version,conntype);
    	}
    	//printf("filename : %s , size : %d \n",filename,st.st_size);
		sendStaticContent(sock,filename,st.st_size,conntype);
		//Send the message back to client
        //write(sock , buffer , strlen(buffer));
		
		//clear the message buffer
		memset(buffer, 0, MAX_BUFFER);
		if(strcasecmp(conntype,"Keep-Alive"))
		{
			break;
		}
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
	}
	close(sock);
	pthread_exit(NULL);
}



void reponseError(int fd, char *errnum,char *shortmsg ,char *longmsg, char *version,char *type) 
{
    char buf[MAXLINE], body[MAX_BUFFER];

    /* Build the HTTP response body */
    sprintf(body, "%s\r\n",longmsg);
    /* Print the HTTP response */
    sprintf(buf, "%s %s %s\r\n", version, errnum, shortmsg);
    write(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    write(fd, buf, strlen(buf));
    sprintf(buf, "Connection: %s\r\n",type);
    write(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    write(fd, buf, strlen(buf));
    write(fd, body, strlen(body));
}


int urlParser(char *url, char *filename) 
{
    char *ptr;
    char pathbuf[64];
    if (!strstr(url, "cgi-bin")) {//it is static content
	strcpy(filename, ".");
	strcat(filename, url);
	if (url[strlen(url)-1] == '/') {
	    sprintf(pathbuf, "%s/index.html", SERVER_ROOT);
	    strcpy(filename, pathbuf);
	}
	else
	{
		sprintf(pathbuf, "%s%s", SERVER_ROOT,url);
	    strcpy(filename, pathbuf);
	}
	return 1;
    }
    return 0;
}

void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
} 

void sendStaticContent(int fd,char *filename,int filesize,char *type)
{
	int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAX_BUFFER];

    /* Send response headers to client */
    get_filetype(filename, filetype);       
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    
    sprintf(buf, "%sConnection: %s\r\n", buf, type);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    write(fd, buf, strlen(buf));

    /* Send response body to client */
    srcfd = open(filename, O_RDONLY, 0);    
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);                           
    write(fd, srcp, filesize);         
	munmap(srcp, filesize); 
}
