#ifndef __J2KCONVERTER__
#define __J2KCONVERTER__

#include <vector>

namespace basicj2k {

class J2kConverter {
 public:
    char* encode(int16_t* data, unsigned int imageWidth, unsigned int imageHeight, size_t& outSize, float distoRatio);
    int16_t* decode(char* inData, size_t inSize, unsigned int& imageWidth, unsigned int& imageHeight, int16_t* outData = nullptr);
    struct MemoryStream {
        size_t offset = 0;
        std::vector<char> data;
    };
};

}

#endif // __J2KCONVERTER__
