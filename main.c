#include "main.h"

#define HOST "0.0.0.0"
#define PORT 8010

#define BUFFER_SIZE 2048

#define HEADER_OK\
    "HTTP/1.1 200 OK\r\n"\
    "Server: AUP-ws\r\n"\
    "Content-Length: %d\r\n\r\n"

#define HEADER_NOT_FOUND\
    "HTTP/1.1 404 NOT FOUND\r\n"

#define DOC_ROOT "/var/www/httpw"

void handle_request(int client_fd, char *msg) {
    int header_len, body_len, fd;
    char body_buf[BUFFER_SIZE], header_buf[512];
    char *path_begin, *path_end;
    int path_len;
    char full_path[256], relative_path[128];

    memset(full_path, 0, sizeof(full_path));
    memset(relative_path, 0, sizeof(relative_path));
    strcpy(full_path, DOC_ROOT);
    path_begin = strchr(msg, '/');
    path_end = strchr(path_begin, ' ');
    path_len = (path_end - path_begin);
    if(path_len > 1) {
        strncpy(relative_path, path_begin, path_len);
    } else {
        strcpy(relative_path, "/index.html");
    }
    strcat(full_path, relative_path);
    /* open the index.html file */
    fd = open(full_path, O_RDONLY);
    if(fd > 0) {
        /* read file contents into buffer */
        body_len = read(fd, body_buf, BUFFER_SIZE);
        printf("Read in index file:\n%s\n\n",body_buf);
        /* clear the header buffer */
        memset(header_buf, 0, sizeof(header_buf));
        /* build the header */
        header_len = sprintf(header_buf, HEADER_OK, body_len);
        printf("Built header:\n%s\n\n",header_buf);
        /* write the header to the client */
        write(client_fd, header_buf, header_len);
        /* write the body to the client */
        write(client_fd, body_buf, body_len);
    } else {
        header_len = sprintf(header_buf, HEADER_NOT_FOUND);
        write(client_fd, header_buf, header_len);
    }
}

int main(int argc, char *argv[])
{
    int baseSocketFd, clientSocketFd, fd_high = 0, fd;
    int nread, i;
    fd_set active_set, read_set;
    struct sockaddr_in my_addr;
    char buf[BUFFER_SIZE];

    /* clear buffer */
    memset(buf, 0, BUFFER_SIZE);
    /* setup a socket connection end-point  */
    baseSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Declaring server socket...\n");
    /* clear structure */
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    /* setup socket address */
    //  inet_addr() converts from dot notation to integer address
    my_addr.sin_addr.s_addr = inet_addr(HOST);
    //  htons() converts to network byte order unsigned small
    my_addr.sin_port = htons(PORT);
    /* bind our HOST / PORT address to the socket file descriptor */
    bind(baseSocketFd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
    printf("Binding server socket...\n");
    /* designate socket as passive. a server */
    listen(baseSocketFd, SOMAXCONN);
    printf("Listening...\n");
    /* zero out active fd set */
    FD_ZERO(&active_set);
    /* add the listening socket to the set */
    FD_SET(baseSocketFd, &active_set);
    /* set the mark for highest fd */
    if(baseSocketFd > fd_high) {
        fd_high = baseSocketFd;
    }

    /* start the servers request / response loop */
    while(1) {
        /* reset the read set (modified by select) */
        read_set = active_set;
        /* see if any sockets are ready */
        select(FD_SETSIZE, &read_set, NULL, NULL, NULL);
        for(i = 0; i < FD_SETSIZE; i++) {
            if(FD_ISSET(i, &read_set)) {
                /* if baseSocket ready for reading we have a new connection */
                if(i == baseSocketFd) {
                    printf("New incoming connection...\n");
                    /* get the new fd for the new connection */
                    clientSocketFd = accept(baseSocketFd, NULL, NULL);
                    printf("Accepting new connection...\n");
                    /* raise the high mark */
                    if(clientSocketFd > fd_high) {
                        fd_high = clientSocketFd;
                    }
                    /* add the new client to our active socket set */
                    FD_SET(clientSocketFd, &active_set);
                    printf("New connection added to socket set...\n");
                }
                /* we're ready to read from a pre-existing connection */
                else {
                    /* read client message */
                    nread = read(i, buf, BUFFER_SIZE);
                    /* check if the connection is closing */
                    if(nread == 0) {
                        printf("Client closing connection...\n");
                        /* close the socket */
                        close(i);
                        /* remove the client socket from our active set */
                        FD_CLR(i, &active_set);
                        /* lower the high mark */
                        fd_high--;
                        printf("Client connection closed...\n");
                    } 
                    /* read client message */
                    else {
                        printf("Client has available message...\n");
                        /* print the client's message */
                        printf("Client Message:\n%s\n\n",buf);
                        handle_request(i, buf);
                        close(i);
                    }
                }
            }
        }
    }
    

    return 0;
}