#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 1024*2

#define CLI_PATH "/var/tmp/odas/"

char * server_path = "/data/DATASETS/ASR/wav2letter.socket";

int main(int argc, char** argv){

    printf("Note: Make sure you have the write access to socket: %s \n", server_path);

    // open input file
    char* fn;
    if(argc == 1){
        fn = "out_0.raw";
    }else if(argc == 2){
        fn = argv[1];
    }else{
        printf("Usage 1: ./uds_client_wav2letter \n ");
        printf("Usage 2: ./uds_client_wav2letter audio_raw_input_filename.raw \n ");
    }

    FILE* in_f = fopen(fn, "rb");
    if(in_f == NULL){
        printf("open %s failed\n", fn);
        exit(-1);
    }

    // unix domain socket
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

    // write into socket 
    size_t block_size = 0;
    while((block_size = fread(buf, sizeof(char), MAXLINE, in_f)) > 0){
        printf("read %ld from file %s\n", block_size, fn);
        int n = write(sockfd, buf, block_size);
        printf("write %d into socket %s\n", n, server_path);
        
        memset(buf_get, 0x00, sizeof(block_size));
    }
    close(sockfd);

    fclose(in_f);

    return 0;

}
