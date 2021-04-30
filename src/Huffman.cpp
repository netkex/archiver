//
// Created by Daniil Pavlenko on 20/04/2021.
//

#include <memory>
#include <set>
#include <iostream>
#include <utility>
#include "Huffman.hpp"

namespace huffman {

void checkInputFileExistence(std::ifstream& in, std::string inputFile) {
    if (!in.is_open())
        throw HuffmanLoadFileException(inputFile);
}

void checkOutputFileExistence(std::ofstream& out, std::string outputFile) {
    if (!out.is_open())
        throw HuffmanLoadFileException(outputFile);
}

std::size_t fileSize(std::string fileName) {
    std::ifstream fileStream(fileName, std::ios::binary);
    checkInputFileExistence(fileStream, fileName);
    const auto begin = fileStream.tellg();
    fileStream.seekg (0, std::ios::end);
    const auto end = fileStream.tellg();
    return end - begin;
}

SizeStatistic code(std::string inputFile, std::string outputFile) {
    if (fileSize(inputFile) == 0) {
        std::ofstream out(outputFile);
        return SizeStatistic{0, 0, 0};
    }
    auto statistic = getStatistic(inputFile);
    auto table = Tree(statistic).getTable();
    std::ofstream out(outputFile);
    checkOutputFileExistence(out, outputFile);
    const auto headerBegin = out.tellp();
    writeHeader(out, outputFile, statistic, table);
    const auto compressedPartBegin = out.tellp();
    writeCompressedFile(inputFile, OutputBitStream(out, outputFile), table);
    const auto endFile = out.tellp();
    return SizeStatistic{fileSize(inputFile),
                         static_cast<std::size_t>(endFile - compressedPartBegin),
                         static_cast<std::size_t>(compressedPartBegin - headerBegin)};
}

SizeStatistic decode(std::string inputFile, std::string outputFile) {
    if (fileSize(inputFile) == 0) {
        std::ofstream out(outputFile);
        return SizeStatistic{0, 0, 0};
    }
    std::ifstream in(inputFile);
    checkInputFileExistence(in, inputFile);
    const auto headerBegin = in.tellg();
    auto header = readHeader(in, inputFile);
    auto tree = std::make_unique<Tree> (header.statistic);
    const auto compressedPartBegin = in.tellg();
    try {
        writeUncompressedFile(InputBitStream(in, inputFile), outputFile, header.size, tree);
    } catch (const HuffmanLogicError& e) {
        throw HuffmanInvalidCompressedFile(inputFile);
    }
    const auto endFile = in.tellg();
    return SizeStatistic{fileSize(outputFile),
                         static_cast<std::size_t>(endFile - compressedPartBegin),
                         static_cast<std::size_t>(compressedPartBegin - headerBegin)};
}

void writeCompressedFile(std::string inputFile, OutputBitStream outStream, const Table& table) {
    std::ifstream in(inputFile);
    checkInputFileExistence(in, inputFile);
    uint8_t byte;
    while (in.read(reinterpret_cast<char *>(&byte), sizeof byte)) {
        outStream.write(table.at(byte));
    }
    outStream.close();
}

void writeUncompressedFile(InputBitStream inStream, std::string outputFile, std::size_t fileSize, const std::unique_ptr<Tree>& tree) {
    std::ofstream out(outputFile);
    checkOutputFileExistence(out, outputFile);
    const Node* curNode = tree->getRoot();
    for (std::size_t pos = 0; pos < fileSize; pos++) {
        curNode = tree->go(curNode, inStream.readBit());
        if (curNode->terminalFlag) {
            uint8_t word = curNode->word;
            curNode = tree->getRoot();
            out.write(reinterpret_cast<char *>(&word), sizeof word);
        }
    }
    if (curNode != tree->getRoot()) {
        throw HuffmanLogicError();
    }
}

Header readHeader(std::ifstream& in, std::string inputFile) {
    Header header;
    std::size_t statisticSize;
    in.read(reinterpret_cast<char *>(&header.size), sizeof header.size);
    in.read(reinterpret_cast<char *>(&statisticSize), sizeof statisticSize);
    if (in.fail())
        throw HuffmanInvalidCompressedFile(inputFile);
    for (std::size_t pos = 0; pos < statisticSize; pos++) {
        uint8_t word;
        std::size_t stat;
        in.read(reinterpret_cast<char *>(&word), sizeof word);
        in.read(reinterpret_cast<char *>(&stat), sizeof stat);
        if (in.fail())
            throw HuffmanInvalidCompressedFile(inputFile);
        header.statistic.emplace_back(word, stat);
    }
    return header;
}

void writeHeader(std::ofstream& out, std::string outputFile, const std::vector<WordStatistic>& statistic, const Table& table) {
    std::size_t fileSize = 0;
    std::size_t statisticSize = statistic.size();
    for (auto wordStat : statistic) {
        fileSize += wordStat.stat * table.at(wordStat.word).size();
    }
    out.write(reinterpret_cast<char *>(&fileSize), sizeof fileSize);
    out.write(reinterpret_cast<char *>(&statisticSize), sizeof statisticSize);
    if (out.fail())
        throw HuffmanWriteFileException(outputFile);
    for (auto wordStat : statistic) {
        out.write(reinterpret_cast<char *>(&wordStat.word), sizeof wordStat.word);
        out.write(reinterpret_cast<char *>(&wordStat.stat), sizeof wordStat.stat);
        if (out.fail())
            throw HuffmanWriteFileException(outputFile);
    }
}

std::vector<WordStatistic> getStatistic(std::string inputFile) {
    std::ifstream in(inputFile);
    checkInputFileExistence(in, inputFile);
    std::size_t rawStatistic[maxByte + 1] = {0};
    uint8_t byte;
    while (in.read(reinterpret_cast<char *>(&byte), sizeof byte)) {
        rawStatistic[byte]++;
    }
    std::vector<WordStatistic> statistic;
    for (int id = 0; id <= maxByte; id++) {
        if (rawStatistic[id] == 0)
            continue;
        statistic.emplace_back(id, rawStatistic[id]);
    }
    return statistic;
}

/** Table realisation */
Table::Table(std::initializer_list<std::pair<uint8_t, std::vector<bool>>> list) {
    for (auto& elem : list) {
        data_[elem.first] = elem.second;
    }
}

std::vector<bool> & Table::operator[](uint8_t id) {
    return data_[id];
}

const std::vector<bool> & Table::at(uint8_t id) const {
    if (data_[id].empty())
        throw HuffmanLogicError();
    return data_[id];
}

bool Table::operator==(const Table &other) const {
    for (std::size_t i = 0; i < maxByte; i++) {
        if (!(data_[i] == other.data_[i]))
            return false;
    }
    return true;
}
/** Table end */

/** Tree realisation */
Tree::Tree(const std::vector<WordStatistic>& statistic) {
    if (statistic.size() == 1) {
        std::unique_ptr<const Node> symbolNode = std::unique_ptr<const Node> (new Node {nullptr, nullptr, true, statistic[0].word});
        root = std::unique_ptr<const Node> (new Node {std::move(symbolNode), nullptr});
        return;
    }
    std::vector<std::unique_ptr<const Node>> pointers;
    std::set<std::pair<std::size_t, std::size_t>> frequencySet; // second parameter is size_t to achieve unambiguity in code and decode mode
    for (auto wordStatistic : statistic) {
        frequencySet.insert({wordStatistic.stat, pointers.size()});
        pointers.push_back(std::unique_ptr<const Node>(new Node {nullptr, nullptr, true, wordStatistic.word}));
    }
    while (frequencySet.size() > 1) {
        auto lessPopularNode1 = *frequencySet.begin();
        frequencySet.erase(lessPopularNode1);
        auto lessPopularNode2 = *frequencySet.begin();
        frequencySet.erase(lessPopularNode2);
        frequencySet.insert({lessPopularNode1.first + lessPopularNode2.first, pointers.size()});
        pointers.push_back(std::unique_ptr<const Node> (new Node{std::move(pointers[lessPopularNode1.second]), std::move(pointers[lessPopularNode2.second])}));
    }
    root = std::move(pointers.back());
}

const Node* Tree::getRoot() const {
    return root.get();
}

const Node* Tree::go(const Node* node, bool bit) {
    if ((bit && node->next1 == nullptr) || (!bit && node->next0 == nullptr))
        throw HuffmanLogicError();
    return (bit ? node->next1 : node->next0).get();
}

Table Tree::getTable() const {
    Table table;
    std::vector<bool> path(0);
    dfs(root.get(), path, table);
    return table;
}

void Tree::dfs(const Node* node, std::vector<bool>& curPath, Table& table) const {
    if (node->terminalFlag) {
        table[node->word] = curPath;
    }
    if (node->next0 != nullptr) {
        curPath.push_back(0);
        dfs(node->next0.get(), curPath, table);
        curPath.pop_back();
    }
    if (node->next1 != nullptr) {
        curPath.push_back(1);
        dfs(node->next1.get(), curPath, table);
        curPath.pop_back();
    }
}
/** Tree end */

}