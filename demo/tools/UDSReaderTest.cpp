#include <iostream>

//#include "demo/tools/UDSReader.h"
#include "UDSReader.h"

const char* input_sss_socket= "/data/DATASETS/ASR/wav2letter_sss.socket";
const char* input_sst_socket= "/data/DATASETS/ASR/wav2letter_sst.socket";

int main(){
    UDSReader uds(input_sss_socket, input_sst_socket);
    
    while(1){
        if(uds.AcceptConn() < 0){
            continue;
        }

        char buffer[32000];
        uds.Read(buffer, 32000);
    }


    return 0;

}
