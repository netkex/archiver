//
// Created by Daniil Pavlenko on 25/04/2021.
//

#ifndef HW_02_BITSTREAM_HPP
#define HW_02_BITSTREAM_HPP

#include <fstream>
#include <vector>
#include <limits>
#include <cinttypes>


namespace huffman {

const int byteBits = std::numeric_limits<uint8_t>::digits;

/** Stream to read from compressed file */
class InputBitStream {
public:
    explicit InputBitStream(std::ifstream &in_, std::string fileName_) : in(in_), fileName(fileName_) {}
    bool readBit();

private:
    uint8_t curByte = 0;
    int cntRead = byteBits;
    std::ifstream &in;
    const std::string fileName;
};

/** Stream to write to compressed file */
class OutputBitStream {
public:
    explicit OutputBitStream(std::ofstream &out_, std::string fileName_) : out(out_), fileName(fileName_) {}

    void writeBit(bool bit);
    void write(const std::vector<bool> &v);
    void close();

private:
    uint8_t curByte = 0;
    int cntWriten = 0;
    std::ofstream &out;
    const std::string fileName;
};

}
#endif //HW_02_BITSTREAM_HPP
