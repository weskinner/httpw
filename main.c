#include "main.h"

#define HOST "0.0.0.0"
#define PORT 8010

#define BUFFER_SIZE 1000

#define HEADER_OK\
    "HTTP/1.1 200 OK\r\n"\
    "Server: AUP-ws\r\n"\
    "Content-Length: 0\r\n"

int main(int argc, char *argv[])
{
    int baseSocketFd, clientSocketFd;
    struct sockaddr_in my_addr;
    char buf[BUFFER_SIZE];

    /* clear buffer */
    memset(buf, 0, BUFFER_SIZE);
    /* setup a socket connection end-point  */
    baseSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    /* clear structure */
    memset(&my_addr, 0, sizeof(struct sockaddr_in));
    /* setup socket address */
    //  inet_addr() converts from dot notation to integer address
    my_addr.sin_addr.s_addr = inet_addr(HOST);
    //  htons() converts to network byte order unsigned small
    my_addr.sin_port = htons(PORT);
    /* bind our HOST / PORT address to the socket file descriptor */
    bind(baseSocketFd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
    /* designate socket as passive. a server */
    listen(baseSocketFd, SOMAXCONN);
    /* accept an incoming connect */
    clientSocketFd = accept(baseSocketFd, NULL, NULL);
    /* read client message */
    read(clientSocketFd, buf, BUFFER_SIZE);
    /* print the client's message */
    printf("Client Message:\n%s\n\n",buf);
    /* write the response message */
    write(clientSocketFd, HEADER_OK, strlen(HEADER_OK));

    return 0;
}