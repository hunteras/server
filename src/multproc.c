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

void sig_child(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        ;//printf("child %d terminated\n", pid);
    return;
}

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
    int	listenfd, connfd, i;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in	cliaddr, servaddr;
    

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

    signal(SIGCHLD, sig_child);

    for ( ;; ) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (SA *) &cliaddr, &clilen);
        if (connfd < 0)
            printf("accept error\n");

        childpid = fork();
        if (childpid == 0) {
            close(listenfd);
            
            http(connfd);
        } else if (childpid < 0) {
            printf("fork error\n");
        } else {
            close(connfd);
        }
    }
}


