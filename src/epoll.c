#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>

#define	LISTENQ	1024	/* 2nd argument to listen() */
#define	MAXLINE	4096	/* max text line length */
#define MAXEVENTS 64
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

}

int main(int argc, char **argv)
{
    int	listenfd, connfd, i;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in	cliaddr, servaddr;
    int efd, s;
    struct epoll_event event;
    struct epoll_event events[MAXEVENTS];

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

    efd = epoll_create1(0);
    if (efd == -1)
    {
        perror ("epoll_create");
        exit(1);
    }

    event.data.fd = listenfd;
    event.events = EPOLLIN;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event);
    if (s == -1)
    {
        perror ("epoll_ctl");
        exit(1);
    }

    for(;;)
    {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);

        for(i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
	    {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                perror("epoll error");
                close (events[i].data.fd);
                continue;
	    } else if (listenfd == events[i].data.fd) {
                /* We have a notification on the listening socket, which
                   means one or more incoming connections. */
                connfd = accept(listenfd, (SA *) NULL, NULL);
                if (connfd < 0)
                    perror("accept error\n");

                event.data.fd = connfd;
                event.events = EPOLLIN;
                s = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);
                if (s == -1)
                {
                    perror ("epoll_ctl");
                    exit(1);
                }
            } else {
                /* We have data on the fd waiting to be read. */
                http(events[i].data.fd);

                close(events[i].data.fd);
            }
        }
    }
}


