//
// Created by Daniil Pavlenko on 20/04/2021.
//

#ifndef HW_02_HUFFMAN_HPP
#define HW_02_HUFFMAN_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <limits>
#include <initializer_list>
#include "BitStream.hpp"
#include "HuffmanException.hpp"

namespace huffman {

const int maxByte = std::numeric_limits<uint8_t>::max();

class Table {
public:
    Table() = default;
    Table(std::initializer_list<std::pair<uint8_t, std::vector<bool>>> list);
    std::vector<bool>& operator[] (uint8_t id);
    const std::vector<bool>& at(uint8_t id) const;
    bool operator == (const Table& other) const; // for tests

private:
    std::vector<bool> data_[maxByte + 1];
};

struct SizeStatistic {
    std::size_t originalSize;
    std::size_t compressedSize;
    std::size_t headerSize;
};

struct WordStatistic {
    explicit WordStatistic(uint8_t word_, std::size_t stat_ = 0) : word(word_), stat(stat_) {}
    uint8_t word;
    std::size_t stat;
};

struct Node {
    const std::unique_ptr<const Node> next0;
    const std::unique_ptr<const Node> next1;
    const bool terminalFlag = false;
    const uint8_t word = 0;
};

class Tree {
public:
    explicit Tree(const std::vector<WordStatistic>& statistic);
    const Node* go(const Node* node, bool bit);
    const Node* getRoot() const;
    Table getTable() const;

private:
    void dfs(const Node* node, std::vector<bool>& curPath, Table& table) const;
    std::unique_ptr<const Node> root;
};

/** Header of compressed file */
struct Header {
    std::size_t size; // compressed file size in bits
    std::vector<WordStatistic> statistic;
};

std::vector<WordStatistic> getStatistic(std::string inputFile); //get byte statistic of file

Header readHeader(std::ifstream& in, std::string inputFile);
void writeHeader(std::ofstream& out, std::string outputFile, const std::vector<WordStatistic>& statistic, const Table& table);

void writeCompressedFile(std::string inputFile, OutputBitStream outStream, const Table& table);
void writeUncompressedFile(InputBitStream inStream, std::string outputFile, std::size_t fileSize, const std::unique_ptr<Tree>& tree);

SizeStatistic code(std::string inputFile, std::string outputFile);
SizeStatistic decode(std::string inputFile, std::string outputFile);

}

#endif //HW_02_HUFFMAN_HPP
