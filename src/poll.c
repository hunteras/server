#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <sys/resource.h>

#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	MAXLINE		4096	/* max text line length */

static int OPEN_MAX = 1024;

typedef struct sockaddr SA;

void http(int fd)
{
    char buff[MAXLINE];
    long ret;
    
    ret = read(fd, buff, MAXLINE); 
    if(ret == 0 || ret == -1) {
        /* printf("read request failed\n"); */
        ;
    }

    sprintf(buff, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>OK</body></html>");
    write(fd, buff, strlen(buff));
    close(fd);

    exit(1);
}

void get_open_max ()
{
    struct rlimit limit;
    if(getrlimit(RLIMIT_NOFILE, &limit) == -1)
        ;
    OPEN_MAX = (int)limit.rlim_cur;
}

int main(int argc, char **argv)
{
    int	i, maxi, listenfd, connfd, sockfd;
    int	nready;
    ssize_t n;
    char buf[MAXLINE];
    socklen_t clilen;
    struct pollfd client[OPEN_MAX];
    struct sockaddr_in cliaddr, servaddr;

    get_open_max();
    
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        printf("create socket error\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(7009);

    if (bind(listenfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        perror("bind error\n");

    if (listen(listenfd, LISTENQ) < 0)
        printf("listen error\n");

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;
    for (i = 1; i < OPEN_MAX; i++)
        client[i].fd = -1;		/* -1 indicates available entry */
    maxi = 0;					/* max index into client[] array */

    for ( ; ; ) {
        nready = poll(client, maxi+1, -1);

        if (client[0].revents & POLLRDNORM) {	/* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

            if (connfd < 0) {
                printf("accept error\n");
                continue;
            }

            for (i = 1; i < OPEN_MAX; i++)
                if (client[i].fd < 0) {
                    client[i].fd = connfd;	/* save descriptor */
                    break;
                }
            if (i == OPEN_MAX) {
                printf("too many clients %d\n", i);
                exit(1);
            }

            client[i].events = POLLRDNORM;
            if (i > maxi)
                maxi = i;				/* max index in client[] array */

            if (--nready <= 0)
                continue;				/* no more readable descriptors */
        }

        for (i = 1; i <= maxi; i++) {	/* check all clients for data */
            if ( (sockfd = client[i].fd) < 0)
                continue;
            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                        /*4connection reset by client */

                        close(sockfd);
                        client[i].fd = -1;
                    } else
                        printf("read error");
                } else if (n == 0) {
                    /*4connection closed by client */
                    close(sockfd);
                    client[i].fd = -1;
                } else {
                    sprintf(buf, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>OK</body></html>");
                    write(sockfd, buf, strlen(buf));

close(sockfd);
                    client[i].fd = -1;                    
                }

                if (--nready <= 0)
                    break;				/* no more readable descriptors */
            }
        }
    }
}
