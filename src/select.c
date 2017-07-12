#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>

#define	LISTENQ		1024	/* 2nd argument to listen() */
#define	MAXLINE		4096	/* max text line length */

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

int main(int argc, char **argv)
{
    int	i, maxi, maxfd, listenfd, connfd, sockfd;
    int	nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

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

    maxfd = listenfd;			/* initialize */
    maxi = -1;					/* index into client[] array */
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;			/* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ; ; ) {
        rset = allset;		/* structure assignment */
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (SA *) &cliaddr, &clilen);

            if (connfd < 0) {
                printf("accept error\n");
                continue;
            }

            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = connfd;	/* save descriptor */
                    break;
                }
            if (i == FD_SETSIZE) {
                printf("too many clients");
                exit(1);
            }

            FD_SET(connfd, &allset);	/* add new descriptor to set */
            if (connfd > maxfd)
                maxfd = connfd;			/* for select */
            if (i > maxi)
                maxi = i;				/* max index in client[] array */

            if (--nready <= 0)
                continue;				/* no more readable descriptors */
        }

        for (i = 0; i <= maxi; i++) {	/* check all clients for data */
            if ( (sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = read(sockfd, buf, MAXLINE)) == 0) {
                    /*4connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    sprintf(buf, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body>OK</body></html>");
                    write(sockfd, buf, strlen(buf));

                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }

                if (--nready <= 0)
                    break;				/* no more readable descriptors */
            }
        }
    }
}

