/**
 * Recieve Audio From ODAS by Unix Domain Socket
 * @author: xiaotaw
 * @email: 
 * @date: 2020/06/08 11:21
 */

#pragma once

#include <sys/socket.h> /* for unix domain socket */
#include <sys/un.h>     /* for unix domain socket */
#include <unistd.h>     /* for unlink */

#include <iostream>
#include <string>
#include <vector>

// params for ODAS 
#define N_CHANNELS 4
// if nBits = 16, BytePerSample=2;
#define BytePerSample 2 
#define HopSize 128

#define SST_BUF_SIZE 1024
#define SSS_BUF_SIZE N_CHANNELS*BytePerSample*HopSize

/**     
 * 
 * [Terminology from ODAS] sss: Source Sound Separate, sst: Source Sound Track
 */
class UDSReader{
  public:
    UDSReader(std::string sss_socket_path, std::string sst_socket_path);
    ~UDSReader();

    int AcceptConn();

    int Read(char* buffer, size_t size);

    int TestSST();

  private:
    /** 
     * create server, bind to socket_path, and start listening.
     * if success, return socket FD, else return failed status
     */
    int CreateServer(std::string socket_path);

    // 
    int AcceptConn(int listenfd);

    void CloseConn();

    // resize m_sss_buf
    // Note: if resised, the existed data would be deleted.
    void ResizeSssBufSize(int new_size);

    // [Act as Server] unix domain socket file description
    int m_sss_listenfd, m_sst_listenfd;

    // connect
    int m_sss_connfd, m_sst_connfd;
    
    // for sss
    char *m_sss_buf;
    int m_sss_buf_size;

    // for sst
    char m_sst_buf[SST_BUF_SIZE];
    std::vector<size_t> m_vec_timestamp;
    std::vector<int> m_vec_idx;
    std::vector<float> m_vec_activity;

};


