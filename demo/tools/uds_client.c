#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 1024

#define CLI_PATH "/var/tmp/odas/"

char * server_path = "/data/odas/demo/tools/server.socket";

int main(){

    struct sockaddr_un cli_un, ser_un;
    int len;
    char buf[MAXLINE];
    char buf_get[MAXLINE];
    int sockfd, n;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
        printf("client socket error\n");
        exit(1);
    }

    memset(&cli_un, 0x00, sizeof(cli_un));
    cli_un.sun_family = AF_UNIX;
    sprintf(cli_un.sun_path, "%s%05d", CLI_PATH, getpid());
    //strcpy(cli_un.sun_path, client_path)
    len = offsetof(struct sockaddr_un, sun_path) + strlen(cli_un.sun_path);

    unlink(cli_un.sun_path);
    if (bind(sockfd, (struct sockaddr *) &cli_un, len) < 0){
        printf("bind error\n");
        exit(1);
    }

    memset(&ser_un, 0x00, sizeof(ser_un));
    ser_un.sun_family = AF_UNIX;
    strcpy(ser_un.sun_path, server_path);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(ser_un.sun_path);
    if (connect(sockfd, (struct sockaddr *) &ser_un, len) < 0){
        printf("connect error\n");
        exit(1);
    }

    while(fgets(buf, MAXLINE, stdin) != NULL){
        write(sockfd, buf, strlen(buf));
        n = read(sockfd, buf_get, MAXLINE);
        if (n < 0){
            printf("the other side has been closed.\n");
        }else if(n == 0){
            printf("get nothing!\n");
        }else{
            write(STDOUT_FILENO, buf_get, n);
            write(STDOUT_FILENO, "\n", 1);
        }
        memset(buf_get, 0x00, sizeof(buf_get));
    }
    close(sockfd);
    return 0;

}
