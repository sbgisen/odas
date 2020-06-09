
//#include "inference/examples/UDSReader.h"
//#include "inference/examples/json.hpp" /*for nlohmann::json*/

//#include "demo/tools/UDSReader.h"
//#include "demo/tools/json.hpp" /*for nlohmann::json*/

#include "UDSReader.h"
#include "json.hpp" /*for nlohmann::json*/

    UDSReader::UDSReader(std::string sss_socket_path, std::string sst_socket_path){
        // create server
        m_sss_listenfd = CreateServer(sss_socket_path);
        m_sst_listenfd = CreateServer(sst_socket_path);

        // allocate buffer
        m_sss_buf = new char[SSS_BUF_SIZE];
        m_sss_buf_size = SSS_BUF_SIZE;

        m_sss_connfd = -1;
        m_sst_connfd = -1;
    }

    UDSReader::~UDSReader(){
        if(m_sss_buf){
            delete[] m_sss_buf;
            m_sss_buf = nullptr;
            m_sss_buf_size = 0;
        }
        // close uds
        close(m_sss_connfd);
        close(m_sst_connfd);
        close(m_sss_listenfd);
        close(m_sst_listenfd);
    }

    int UDSReader::AcceptConn(){
        if(m_sss_connfd < 0){
            std::cout << "accept sss" << std::endl;
            m_sss_connfd = AcceptConn(m_sss_listenfd);
        }
        std::cout << "m_sss_connfd： " << m_sss_connfd << std::endl;
        if(m_sst_connfd < 0){
            std::cout << "accept sst" << std::endl;
            m_sst_connfd = AcceptConn(m_sst_listenfd);
        }
        std::cout << "m_sst_connfd： " << m_sst_connfd << std::endl;
        return (m_sss_connfd >= 0) && (m_sss_connfd >= 0);
    }

    void UDSReader::CloseConn(){
        close(m_sss_connfd);
        close(m_sst_connfd);
    }
    
    /**
     * Read some audio into buffer, with a length of size.
     * Note:
     *   1. the number of samples = size / BytePerSample.
     *   2. As the audio come from ODAS with multi-channels, 
     *      we choose the channel with highest activity in each audio sample.
     * Return value < 0, if failed.
     */
    int UDSReader::Read(char* buffer, size_t size){
        // check 
        //std::cout << "size: " << size << std::endl;
        if(size % (BytePerSample * HopSize) ){
            std::cerr << "The size of to be read, should be devided by BytePerSample * HopSize. ";
            std::cerr << "But got " << size << " vs " << BytePerSample * HopSize << std::endl;
            return -1;
        }

        // resize sst info buf
        int n_samples = size / (BytePerSample * HopSize);
        m_vec_timestamp.resize(n_samples);
        m_vec_idx.resize(n_samples);
        m_vec_activity.resize(n_samples);
        
        int n_bytes;
        for(int i=0; i<n_samples; i++){
            //std::cout << "read " << i << "th sample" << std::endl;
            // read sst data
            memset(m_sst_buf, 0x00, SST_BUF_SIZE);
            int n_bytes = read(m_sst_connfd, m_sst_buf, SST_BUF_SIZE);
            if(n_bytes < 0){
                std::cerr << "read data from sst error" << std::endl;
                return n_bytes;
            }
            //std::cout << "read data from sst: " << n_bytes << std::endl;
            
            // find max activity channel
            nlohmann::json sst_j = nlohmann::json::parse(std::string(m_sst_buf));
            int max_idx = 0;
            float max_activity = 0.0;
            // the output of sst from odas has 4 tracks, 
            // we find out the track with highest activity
            for(int j = 0; j < 4; j++){
                float activity = sst_j["src"][j]["activity"];
                if ( activity > max_activity){
                    max_idx = j;
                    max_activity = activity;
                }
            }
			m_vec_timestamp[i] = sst_j["timeStamp"];
			m_vec_idx[i] = max_idx;
			m_vec_activity[i] = max_activity;
            //std::cout << "read sst: " << i << " timeStamp: " << sst_j["timeStamp"] << std::endl;

            // read sss data
            n_bytes = read(m_sss_connfd, m_sss_buf, SSS_BUF_SIZE);
            if(n_bytes < 0){
                std::cerr << "read data from sss error" << std::endl;
                return n_bytes;
            }else if(n_bytes != SSS_BUF_SIZE){
                std::cerr << "read buf size not complete!" << std::endl;
                return 0;
            }else{
                //std::cout << "read data from sss: " << n_bytes << std::endl;
                // deal with HopSize samples
                int buffer_offset = i * (BytePerSample * HopSize);
                // we only pick the channel with the highest activity for each sample
                int max_activity_idx = m_vec_idx[i];
                for(int j=0; j<HopSize; j++){
                    for(int k=0; k<BytePerSample; k++){
                        buffer[buffer_offset + j * BytePerSample + k] = 
                            m_sss_buf[(j * N_CHANNELS + max_activity_idx) * BytePerSample + k];
                    }
                }
            }
            //std::cout << "read sss: " << HopSize << std::endl;
        }
        return n_samples * BytePerSample * HopSize;


        // int n = read(m_sss_connfd, m_sss_buf, N_CHANNELS * size);
        // if(n < 0){
        //     std::cerr << "read error" << std::endl;
        //     return n;
        // }else if(n % N_CHANNELS){
        //     std::cerr << "read buf size should be devided by number of channels" << std::endl;
        //     n = 0;
        //     return n;
        // }else if((n / N_CHANNELS) % BytePerSample){
        //     std::cerr << "read buf size should be devided by number of BytePerSample" << std::endl;
        //     n = 0;
        //     return n;
        // }else{
        //     // (xt) TODO: read sss, calculate n_samples, then read sst data.
        //     int n_samples = n / (N_CHANNELS * BytePerSample);
        //     char *p = m_sss_buf, *q = buffer;
        //     for(int i=0; i<n_samples; i++){
        //         for(int j=0; j<BytePerSample; j++){
        //             int idx = m_vec_idx[i];
        //             *(q + j) = *(p + idx * BytePerSample + j);
        //         }
        //         p += N_CHANNELS * BytePerSample ;
        //         q += BytePerSample;
        //     }
        //     return n_samples * BytePerSample;
        // }
    }

    void UDSReader::ResizeSssBufSize(int new_size){
        if(new_size > m_sss_buf_size){
            delete[] m_sss_buf;
            m_sss_buf = new char[new_size];
            m_sss_buf_size = new_size;
        }
    }


    int UDSReader::CreateServer(std::string socket_path){
      // remove socket file if exists
      unlink(socket_path.c_str());

      // create server
      int listenfd = -1;
      if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0){
          std::cerr << "create new socket error!" << std::endl;
          return listenfd;
      }

      // init socket address
      struct sockaddr_un ser_un;
      memset(&ser_un, 0x00, sizeof(ser_un));
      ser_un.sun_family = AF_UNIX;
      strcpy(ser_un.sun_path, socket_path.c_str());

      int res = 0;

      // bind server to socket_path
      int size = offsetof(struct sockaddr_un, sun_path) + strlen(ser_un.sun_path);
      if((res = bind(listenfd, (struct sockaddr *)&ser_un, size)) < 0 ){
          std::cerr << "socket file: " << socket_path << ", bind error!" << std::endl;
          return res;
      }

      // listen 
      if((res = listen(listenfd, 20)) < 0){
          std::cerr << "socket file: " << socket_path << ", listen error!" << std::endl;
          return res;
      }
      return listenfd;
    }

    int UDSReader::AcceptConn(int listenfd){
        struct sockaddr_un cli_un;
        socklen_t cli_un_len = sizeof(cli_un);
        int connfd = accept(listenfd, (struct sockaddr *)&cli_un, &cli_un_len);
        if(connfd < 0){
            std::cerr << "accept error" << std::endl;
        }
        return connfd;
    }


    /**
     * Note thta uds may block if full with data.
     */
    int UDSReader::TestSST(){
        while(1){
            if((m_sst_connfd = AcceptConn(m_sst_listenfd)) < 0){
                continue;
            }
            while(1){
                // resize sst info buf
                int n_samples = 1600;
                m_vec_timestamp.resize(n_samples);
                m_vec_idx.resize(n_samples);
                m_vec_activity.resize(n_samples);

                int n_bytes;
                for(int i=0; i<n_samples; i++){
                    // read sst data
                    std::cout << "try to read sst: " << i << std::endl;
                    memset(m_sst_buf, 0x00, SST_BUF_SIZE);
                    int n_bytes = read(m_sst_connfd, m_sst_buf, SST_BUF_SIZE);
                    if(n_bytes < 0){
                        std::cerr << "read data from sst error" << std::endl;
                        return n_bytes;
                    }
                    // find max activity channel
                    nlohmann::json sst_j = nlohmann::json::parse(std::string(m_sst_buf));
                    int max_idx = 0;
                    float max_activity = 0.0;
                    // the output of sst from odas has 4 tracks, 
                    // we find out the track with highest activity
                    for(int j = 0; j < 4; j++){
                        float activity = sst_j["src"][j]["activity"];
                        if ( activity > max_activity){
                            max_idx = j;
                            max_activity = activity;
                        }
                    }
                    m_vec_timestamp[i] = sst_j["timeStamp"];
                    m_vec_idx[i] = max_idx;
                    m_vec_activity[i] = max_activity;
                    std::cout << "read " <<  n_bytes << " from sst, timeStamp: " << sst_j["timeStamp"] << std::endl;
                }
            }
        }

    }

