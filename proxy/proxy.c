#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h> 
#include <errno.h>
#include <time.h>

/* const */
int RESPONSE_BUFFER_SIZE = 5000;
int GET_BUFFER_SIZE = 2000;
int HOST_BUFFER_SIZE = 100;

FILE *LOG_FILE;
/* request struct */
struct request {
    int port;
    char *host;
    char *get;
    struct sockaddr_in *socket;
};

/* handler functions */
void server_loop(int sockfd);
void handle_request(int client_sockfd, struct sockaddr_in *sockaddr);
int foward_response(struct request *request, int client_sockfd);

/* factory functions */
struct request *request_factory();
void free_request(struct request **request);
struct sockaddr_in *sockaddr_factory(int port);
struct sockaddr_in *sockaddr_server_factory(struct request *request);
void free_sockaddr(struct sockaddr_in **sockaddr);
int socket_factory();

/* helper functions */
int end_of_request(const char *buffer, int buffer_size);
int is_get_request(struct request *request);
int extract_hostname(struct request *request);
int extract_url(struct request *request);
int extract_port(struct request *request, char *port_char);
int get_content_length(const char *response);
int get_http_status(const char *response);
void validate_argc(int argc);
int validate_port(const char *port);

/* error function */
void error(const char *message);

/**
 * handles signals from children
 * @param sig {int}
 */
void sigchld_handler(int sig) {
    int status;
    printf("running sigchld_handler[%d]\n", sig);
    while (waitpid((pid_t)(-1), &status, WNOHANG) > 0) {
        printf("reaped[%d]\n", status);
    }
}

/**
 * taken from csapp.c
 * @param  port {char *}
 * @return      {int}
 */
int open_listenfd(char *port) 
{
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval=1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
                   (const void *)&optval , sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        if (close(listenfd) < 0) { /* Bind failed, try the next */
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }


    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 1024) < 0) {
        close(listenfd);
        return -1;
    }
    return listenfd;
}

int main(int argc, char **argv) {
    int sockfd;
    int port;
    struct sockaddr_in *server_address;

    /* REAP ALL THE DEAD CHILDREN */
    signal(SIGCHLD, sigchld_handler);

    /* validate argument count */
    validate_argc(argc);

    /* validate port */
    port = validate_port(argv[1]);

    sockfd = open_listenfd(argv[1]);

    printf("SOCKET NUM [%d]\n", sockfd);

    printf("Listening to port %d\n", port);

    /* infinite loop */
    server_loop(sockfd);

    /* free server address */
    free_sockaddr(&server_address);

    return 0;
}

/**
 * main server look
 * @param sockfd {int} socket file descriptor
 */
void server_loop(int sockfd) {
    struct sockaddr_in client_address;
    int client_sockfd;
    socklen_t client_len = sizeof(client_address);

    for(;;) {
        /* accept the incoming socket */
        client_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_len);

        /* check if socket has any errors, if so it exits(1) */
        if (client_sockfd < 0)
            error("Error with client accepting socket\n");

        /* fork the process and let the child handle the request */
        if (fork() == 0) {
            /* child */
            close(sockfd);
            handle_request(client_sockfd, &client_address);
        } else {
            /* parent */
            close(client_sockfd);
        }
    }
}

/**
 * handles the clients request
 * @param client_sockfd {int} socket file descriptor
 */
void handle_request(int client_sockfd, struct sockaddr_in *sockaddr) {
    int size;
    struct request *request = request_factory();
    char *request_ptr = request->get;
    request->socket = sockaddr;


    /* reads request until \r\n\r\n */
    while((size = read(client_sockfd, request_ptr, GET_BUFFER_SIZE)) > 0) {
        request_ptr += size;
        printf("%s\n", request->get);
        if (end_of_request(request->get, strlen(request->get)))
            break;
    }

    if (size < 0)
        error("Error reading from socket \n");

    if (is_get_request(request)) {
        printf("=== REQUEST ===\n");
        printf("%s\n", request->get);

        if ( ! extract_hostname(request))
            extract_url(request);

        printf("[%s][%d]\n", request->host, request->port);
        foward_response(request, client_sockfd);
    } else {
        printf("-- WAS NOT GET REQUEST --");
    }

    close(client_sockfd);

    free_request(&request);

    /* child exits */
    exit(0);
}

/**
 * sends request over to the server and fowards the 
 * servers response to the client
 * @param  request       {struct request *}
 * @param  client_sockfd {sock file descriptor}
 * @return               {boolean}
 */
int foward_response(struct request *request, int client_sockfd) {
    int server_sockfd, n, buffer_size, http_status;
    /* content-length: dual purpose, flag and acutal count*/
    int content_length = 0;
    /* first is a usngined int use as boolean flag for first read */
    unsigned char first = 1;
    unsigned char kill = 0;
    struct sockaddr_in *server_address;
    char buffer[RESPONSE_BUFFER_SIZE];
    time_t timer;
    char current_time[30];
    struct tm* tm_info;
    int total_bytes = 0;

    server_sockfd = socket_factory();
    server_address = sockaddr_server_factory(request);

    if (connect(server_sockfd, (struct sockaddr *) server_address, sizeof(struct sockaddr_in)) < 0)
        error("Error connecting to server\n");

    n = write(server_sockfd, request->get, strlen(request->get));

    if (n < 0)
        error("Error writing to socket");

    bzero(buffer, RESPONSE_BUFFER_SIZE);

    /* read from web server */
    while((buffer_size = read(server_sockfd, buffer, RESPONSE_BUFFER_SIZE)) > 0) {
        
        /* if buffer length is less than or equal to zero exit */
        if (buffer_size <= 0)
            break;
        total_bytes += buffer_size;

        /* checks if content legnth is set, and adds buffer */
        if (first && (content_length = get_content_length(buffer)) != 0) {
            printf("- --------- CONTENT LENGTH -----------");
            content_length += strstr(buffer, "\r\n\r\n") - buffer;
        }

        /* disable first flag*/
        first = 0;

        printf(" size [%d]\n", buffer_size);
        /* write to clients browser */
        write(client_sockfd, buffer, buffer_size);
        printf("%s\n", buffer);

        /* if kill flag is set or content-length is 0 or we get \r\n\r\n we exit */
        if (kill || (content_length && (content_length -= buffer_size) <= 0) || (end_of_request(buffer, buffer_size))) {
            printf("[[[[ KILLED ]]]]\n");
            break;
        }

        bzero(buffer, RESPONSE_BUFFER_SIZE);

    }
    
    /* logging portion */
    time(&timer);
    tm_info = localtime(&timer);

    char char_ip[100];

    unsigned long ip;
    //struct in_addr ip;
    ip = request->socket->sin_addr.s_addr;

    printf("%lu\n", ip);
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;   
    sprintf(char_ip,"%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

    printf("%s\n", char_ip);
    strftime(current_time, 30, "%a %d %b %G %T %Z", tm_info);

    puts(current_time);
    LOG_FILE = fopen("proxy.log", "a");
    fprintf(LOG_FILE, "%s: %s %s %d\n", current_time, char_ip, request->host, total_bytes);
    //fputs("This is testing for fputs...\n", LOG_FILE);
    fclose(LOG_FILE);
    return close(server_sockfd);
}

/**
 * checks to see if end char * contains \r\n\r\n sequence
 * @param  buffer      {const char *}
 * @param  buffer_size {int}
 * @return             {boolean}
 */
int end_of_request(const char *buffer, int buffer_size) {
    return (strcmp(&buffer[buffer_size - 4], "\r\n\r\n") == 0);
}

/**
 * checks if request has a GET method
 * @param  request {struct request *}
 * @return         {boolean}
 */
int is_get_request(struct request *request) {
    return (strstr(request->get, "GET") != 0);
}

/**
 * request factory
 * @return {struct request *}
 */
struct request *request_factory() {
    struct request *request = calloc(1, sizeof(struct request));
    request->port = 80;
    request->get = calloc(GET_BUFFER_SIZE, sizeof(char));
    request->host = calloc(HOST_BUFFER_SIZE, sizeof(char));
    return request;
}

/**
 * free's the request pointer and its variables
 * @param request {struct request **}
 */
void free_request(struct request **request) {
    free((*request)->get);
    (*request)->get = NULL;
    free((*request)->host);
    (*request)->host = NULL;
    free(*request);
    (*request) = NULL;
}

/**
 * creates a new socketaddr struct and returns
 * the pointer
 * 
 * @param  port {int}
 * @return      {struct sockaddr_in *}
 */
struct sockaddr_in *sockaddr_factory(int port) {
    struct sockaddr_in *sockaddr = calloc(1, sizeof(struct sockaddr_in));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = INADDR_ANY;
    sockaddr->sin_port = htons(port);
    return sockaddr;
}

/**
 * frees sockaddr struct
 * @param sockaddr {free_sockaddr}
 */
void free_sockaddr(struct sockaddr_in **sockaddr) {
    free(*sockaddr);
    (*sockaddr) = NULL;
}

/**
 * creates a new socket
 * @return {int} socket file descriptor
 */
int socket_factory() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sock < 0)
        error("Error opening socket\n");

    return sock;
}

/**
 * creates a new sockaddr_in struct
 * @param  request {struct request *}
 * @return         {struct sockaddr_in *}
 */
struct sockaddr_in *sockaddr_server_factory(struct request *request) {
    struct hostent *server;
    struct sockaddr_in *server_addr = calloc(1, sizeof(struct sockaddr_in));

    server = gethostbyname(request->host);

    /* check if host is reachable */
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        free_request(&request);
        exit(0);
    }

    /* set up sockaddr_in struct */
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(request->port);
    memcpy(&(server_addr->sin_addr.s_addr), server->h_addr, server->h_length);

    return server_addr;
}

/**
 * extracts hostname from url
 * @param  request {struct request *}
 * @return         {int} boolean
 */
int extract_hostname(struct request *request) {
    char *start;
    char *end;
    char *port;

    start = strstr(request->get, "Host: ");
    
    /* if start is null, host is not set. */
    if (start == NULL)
        return 0;

    /* move 6 chars over to get domain name */
    start += 6;

    /* find the first \r\n after `Host: ` */
    end = strstr(start, "\r\n");

    if (end == NULL) {
        printf("End char went wrong\n");
        return 0;
    }
    /* copy from end of Host: - to begining of \r\n */
    strncpy(request->host, start, end - start);

    /* looking to see if port is set */
    port = strstr(request->host, ":");
    
    /* if port is found, extract port to request object */
    if (port != NULL)
        extract_port(request, port);

    /* typically we calloc everything to make it zero */
    /* but incase one slips by */
    request->host[end - start] = '\0';

    return 1;
}

/**
 * extracts the url from GET request
 * @param  request {struct request *}
 * @return         {int} boolean
 */
int extract_url(struct request *request) {
    int offset = 7;
    char *start;
    char *end;

    if ((start = strstr(request->get, "http://")) == NULL) {
        if ((start = strstr(request->get, "https://")) == NULL) {
            /* your fucked */
            return 0;
        }
        offset = 8;
    }

    start += offset;

    end = strstr(start, "/");
    
    /* if end is null this is some werid malformed char */
    if (end == NULL)
        return 0;

    /* sanity checks */
    if (start > end) {
        printf("Abort trap 6\n");
        return 0;
    }

    strncpy(request->get, start, end - start);

    /* typically we calloc everything but just in case one gets by */
    request->get[end - start] = '\0';

    return 1;
}

/**
 * extracts a port if a portnumber is preset
 * @param  request   {struct request *}
 * @param  port_char {char *} port
 * @return           {int} boolean
 */
int extract_port(struct request *request, char *port_char) {
    printf("extracting port\n");
    char *port = port_char;
    char *temp_port = calloc(6, sizeof(char));
    
    *port_char = '\0';
    /* move pointer over by 1; */
    port++;

    strcpy(temp_port, port);

    request->port = atoi(temp_port);

    if (request->port == 0)
        request->port = 80;

    free(temp_port);

    return 1;
}

/**
 * looks for the content length and returns a number if found
 * 
 * @param  response {const char *}
 * @return          {int} boolean
 */
int get_content_length(const char* response) {
    char *start;
    char *end;
    char length[25];
    int len;

    char content_len[] = "Content-Length: ";

    start = strstr(response, content_len);

    if (start == NULL)
        return 0;

    start += strlen(content_len);

    end = strstr(start, "\r\n");

    if (end == NULL)
        return 0;

    strncpy(length, start, end - start);
    length[end - start] = '\0';
    len = atoi(length);
    return len;
}

/**
 * gets the response http response
 * @param  response {const char *}
 * @return          {int boolean}
 */
int get_http_status(const char *response) {
    char status[4];
    strncpy(status, response+9, 3);
    status[3] = '\0';

    printf("status [%s]\n", status);
    return atoi(status);
}

/**
 * validates arg count
 * @param argc {int}
 */
void validate_argc(int argc) {
    if (argc == 1)
        error("PORT is not specified\n");

    if (argc > 2)
        error("hmm too many args?\n");
}

/**
 * validates port number
 * @param  port {const char *}
 * @return      {int} port number
 */
int validate_port(const char *port) {
    int port_number = atoi(port);

    if (1024 > port_number || port_number > 65536)
        error("Port is out of range\n");

    return port_number;
}

/**
 * prints out an error message then exits(1)
 * @param message {const char *} error msg
 */
void error(const char *message) {
    printf("%s\n",message);
    exit(1);
}



