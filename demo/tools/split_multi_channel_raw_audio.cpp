/**
 * read multi-channel audio raw data, and split into seperate files(mono channel)
 * @author: xiaotaw
 * @email: 
 * @date: 2020/06/05 08:01
 */

#include <iostream>
#include <fstream>
#include <vector>

struct WavHead{
    char RIFF[4];    //头部分那个RIFF
    int32_t size0;  //存的是后面所有文件大小
    char WAVE[4];
    char FMT[4];
    int32_t size1;  // 存的是fmt保存的大小，包含这之后，data前面几个，共16个
    int16_t fmttag;  // tag, 固定为1
    int16_t channel;        // 声道数
    int32_t samplespersec;  //每秒采样数，16000 or 32000 or other
    int32_t bytepersec;
    int16_t blockalign;     // channel * bit_per_samples / 8
    int16_t bitpersamples;  // 8, or 16, or others
    char DATA[4];
    int32_t size2;  //剩下文件大小，也就是声音采样是大小
};
WavHead head={ {'R','I','F','F'}, 
                44 - 8,     /* size after */
                {'W','A','V','E'},
                {'f','m','t',' '},
                16,
                1,     /* fmttag */
                1,     /* channel*/
                16000, /* sample rate */
                32000, /* byte per sec */
                2,     /* block align units */
                16,    /* bit per samples */
                {'d','a','t','a'},
                0      /* size after */
            };


int main(){
    // infomation about input file
    size_t fs = 16000;
    size_t hop_size = 128;
    unsigned n_bits = 16;
    unsigned n_channels = 4;
    float gain = 10.0;
    std::string fn = "/data/odas/bin/postfiltered.raw";

    // open audio file
    std::ifstream in_f(fn, std::ios_base::binary|std::ios_base::in);
    if(!in_f){
        std::cerr << fn << " does not exists!" << std::endl;
        exit(-1);
    }

    // get length of the audio file
    in_f.seekg(0, std::ios::end);
    size_t f_len = in_f.tellg();
    std::cout << "length of " << fn << ": " << f_len << std::endl;
    in_f.clear();
    in_f.seekg(0);


    // trying to split multi-channels into mono channels
    std::vector<std::ofstream *> out_fs;
    for(int i=0; i<n_channels; i++){
        char tmp_c[20];
#define SAVE_WAV
#ifdef SAVE_WAV
        sprintf(tmp_c, "out_%d.wav", i);
#else
        sprintf(tmp_c, "out_%d.raw", i);
#endif
        std::ofstream*  out_f = new std::ofstream(tmp_c, std::ios_base::binary|std::ios_base::out);
        out_fs.push_back(out_f);
#ifdef SAVE_WAV
        head.size0 += f_len / n_channels;
        head.size2 += f_len / n_channels;
        (*out_f).write(reinterpret_cast<char*>(&head), sizeof(WavHead));
#endif
    }

    // 16 bits = 2 Bytes = 2 char = 1 short
    std::cout << "sizeof short: " << sizeof(short) << std::endl;
    std::cout << "wav head size: " << sizeof(WavHead) << std::endl;

    size_t size_per_block = 1024 * n_channels * sizeof(short);
    std::vector<char> buffer;
    buffer.resize(size_per_block); 

    while(in_f.good()){
        // read a block
        in_f.read(buffer.data(), size_per_block);
        size_t l = in_f.gcount();

        if(l % (n_channels * sizeof(short)) != 0 ){
            std::cerr << "read data is not complete" << std::endl;
        }
        
        // split multi channel into mono channel, and write into file
        size_t offset = 0;
        for(int i = 0; i < l / (n_channels * sizeof(short)); i++){
            for(int j = 0; j < n_channels; j++){
                (*out_fs[j]).write(buffer.data() + offset, sizeof(short));
                offset += sizeof(short);
            }
        }
    }

    // close mono-channel output file 
    for(int i=0; i<n_channels; i++){
        (*out_fs[i]).close();
        delete out_fs[i];
        out_fs[i] = nullptr;
    }

    return 0;
}
