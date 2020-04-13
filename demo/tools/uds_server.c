#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
//#include <errno.h>
#include <string.h> /* for strcpy, strlen */
#include <unistd.h> /* for unlink */
//#include <ctype.h>

#define MAXLINE 1024

char *socket_path = "server.socket";

int main(void){

    struct sockaddr_un ser_un, cli_un;
    socklen_t cli_un_len;
    int listenfd, connfd, size;
    char buf[MAXLINE];
    int i, n;

    if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
        printf("socket error!\n");
        exit(1);
    }

    memset(&ser_un, 0x00, sizeof(ser_un));
    ser_un.sun_family = AF_UNIX;
    strcpy(ser_un.sun_path, socket_path);
    size = offsetof(struct sockaddr_un, sun_path) + strlen(ser_un.sun_path);
    unlink(socket_path);
    if (bind(listenfd, (struct sockaddr *)&ser_un, size) < 0){
        printf("bind error!\n");
        exit(1);
    }else{
        printf("unix domain socket bound.\n");
    }

    if (listen(listenfd, 20) < 0){
        printf("listen error\n");
        exit(1);
    }


    while(1){
        cli_un_len = sizeof(cli_un);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cli_un, &cli_un_len)) < 0){
            printf("accept error\n");
            continue;
        }

        while(1){
            n = read(connfd, buf, sizeof(buf));
            if(n < 0){
                printf("read error\n");
                break;
            }else if(n == 0){
                printf("EOF\n");
                break;
            }
            printf("received size: %d\n", n);

            // send back to client: the first 5 char
            write(connfd, buf, 5);
            memset(buf, 0x00, sizeof(buf));
        }
        close(connfd);
    }
    close(listenfd);
    return 0;
}


