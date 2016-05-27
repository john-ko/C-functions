#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

char test[] = "GET / HTTP/1.1\nHOST: www.google.com\n\n";


void error(const char *message);


// gcc -lpthread 
int validate_port(const char *port);
int validate_args(int argc);
int socket_factory();
void setup_sockaddr(struct sockaddr_in *sockaddr_in, int PORT);

void sigchld_handler(int sig) {
	int status;
	printf("running sigchld_handler[%d]\n", sig);
	while (waitpid((pid_t)(-1), &status, WNOHANG) > 0) {
		printf("reaped[%d]\n", status);
	}
}

int handle_request(int sockfd, int newsockfd) {

  // close listener socket
  close(sockfd);
  char buffer[256];
  int size;

  while(1) {
    bzero(buffer,256);
    size = read(newsockfd,buffer,255);
    printf("did read worked?\n");
    if (size < 0) {
      error("error reading from socket \n");
    }
    printf("buffer[%s]\n", buffer);
    printf("id%d\n", newsockfd);
    printf("n[%d]\n", size);
    printf("id[%d] size[%d] msg: %s\n",newsockfd, size, buffer);

    printf("flsuh buffer?\n");

    if (strcmp(buffer, "quit\r\n") == 0) {
      close(newsockfd);
      return newsockfd;
    }
  }
}

int main(int argc, char* argv[]) {
  int sockfd;
  int newsockfd;
  int PORT;
  int n;
  char buffer[256];
  pid_t pid;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;


  signal(SIGCHLD, sigchld_handler);

  // use validate args
  validate_args(argc);

  // validate port
  PORT = validate_port(argv[1]);

  // create socket
  sockfd = socket_factory();

  // zero out bits in the struct
  bzero((char *) &serv_addr, sizeof(serv_addr));
  
  // set up serv_addr struct
  setup_sockaddr(&serv_addr, PORT);
  
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    error("Error in binding socket.. soz\n");
  }
  
  // listen to socket
  listen(sockfd, 5);
  printf("Listening...\n");


  while(1) {
    printf("awaiting connection\n");
    // accept a new socket connection
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    printf("connected!");

    // error checking on socket
    if (newsockfd < 0) {
      error("Error on accept... soz\n");
    }
  
    // fork a new processes
    pid = fork();
    if (pid == 0) {
      printf("forked!");
      handle_request(sockfd, newsockfd);
    } else {
      // parent should close connection
      close(newsockfd);
    }
  }
  close(sockfd);
  return 0;
}

int validate_args(int argc) {
  if (argc == 1) {
    error("PORT not specified\n");
  }

  if (argc > 2) {
    error("lol too many args\n");
  }

  return 1;
}

int validate_port(const char *port) {

  int port_number = atoi(port);

  if (1024 > port_number || port_number > 65536) {
    error("port out of range\n");
  }

  return port_number;
}

int socket_factory() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    error("Error opening socket\n");
  }
  
  return sock;
}

void setup_sockaddr(struct sockaddr_in *serv_addr, int PORT) {
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_addr.s_addr = INADDR_ANY;
  serv_addr->sin_port = htons(PORT);
}

void error(const char *message) {
  printf(message);
  exit(1);
}



