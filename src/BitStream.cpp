//
// Created by Daniil Pavlenko on 25/04/2021.
//

#include "BitStream.hpp"
#include "HuffmanException.hpp"

namespace huffman {

bool InputBitStream::readBit() {
    if (cntRead == byteBits) {
        if (!in.read(reinterpret_cast<char *>(&curByte), sizeof curByte))
            throw HuffmanInvalidCompressedFile(fileName);
        cntRead = 0;
    }
    return (curByte >> (cntRead++)) & 1;
}

void OutputBitStream::writeBit(bool bit) {
    if (cntWriten == byteBits) {
        if (!out.write(reinterpret_cast<char *>(&curByte), sizeof curByte))
            throw HuffmanWriteFileException(fileName);
        curByte = 0;
        cntWriten = 0;
    }
    curByte |= (bit * (1 << (cntWriten++)));
}

void OutputBitStream::write(const std::vector<bool> &v) {
    for (bool x : v) {
        writeBit(x);
    }
}

void OutputBitStream::close() {
    if (!out.write(reinterpret_cast<char *>(&curByte), sizeof curByte))
        throw HuffmanWriteFileException(fileName);
}

}