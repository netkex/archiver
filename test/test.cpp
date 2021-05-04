//
// Created by Daniil Pavlenko on 19/04/2021.
//

/**
 * use #include <doctest/doctest.h> indeed "doctest.h", because clion doesn't make run button for each HTest in second variant
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "Huffman.hpp"

using namespace huffman;

std::string pathToResources(std::string fileName) {
    return "resources/" + fileName;
}

bool statisticEq(const SizeStatistic& statistic1, const SizeStatistic& statistic2) {
    return (statistic1.headerSize == statistic2.headerSize && statistic1.compressedSize == statistic2.compressedSize
           && statistic1.originalSize == statistic2.originalSize);
}

void codeAndDecodeCheck(std::string fileName, bool specialStatisticCheck = false, SizeStatistic specialStatistic = SizeStatistic {0, 0, 0}) {
    auto codeStatistic = code(pathToResources(fileName), pathToResources("testTmp.txt"));
    auto decodeStatistic = decode(pathToResources("testTmp.txt"), pathToResources("out.txt"));
    CHECK(system(("diff " + pathToResources(fileName) + " " + pathToResources("out.txt")).c_str()) == 0);
    CHECK(statisticEq(codeStatistic, decodeStatistic));
    if (specialStatisticCheck) {
        CHECK(statisticEq(codeStatistic, specialStatistic));
    }
    system(("rm " + pathToResources("testTmp.txt")).c_str());
    system(("rm " + pathToResources("out.txt")).c_str());
}

TEST_CASE("Exception: nonexistent file") {
    CHECK_THROWS_AS(code("nonexist", pathToResources("sample_01.txt")), const HuffmanLoadFileException&);
    CHECK_THROWS_AS(decode("nonexist", pathToResources("sample_01.txt")), const HuffmanLoadFileException&);
}

TEST_CASE("Exception: invalid compressed file") {
    CHECK_THROWS_AS(decode(pathToResources("sample_01.txt"), pathToResources("testTmp.txt")), const HuffmanInvalidCompressedFile&);
}

TEST_CASE("Tree") {
    Tree tree1(std::vector<WordStatistic> {WordStatistic('a', 5), WordStatistic('b', 3), WordStatistic('c', 4)});
    Tree tree2(std::vector<WordStatistic> {WordStatistic('A', 100)});

    SUBCASE("constructor/tree structure") {
        SUBCASE("tree1") {
            const Node* root = tree1.getRoot();
            CHECK(root->next0 != nullptr);
            CHECK(root->next1 != nullptr);
            CHECK(root->next0->next0 == nullptr);
            CHECK(root->next0->next1 == nullptr);
            CHECK(root->next1->next0 != nullptr);
            CHECK(root->next1->next1 != nullptr);
            CHECK(root->next1->next0->next0 == nullptr);
            CHECK(root->next1->next0->next1 == nullptr);
            CHECK(root->next1->next1->next0 == nullptr);
            CHECK(root->next1->next1->next1 == nullptr);
        }
        SUBCASE("tree2") {
            const Node* root = tree2.getRoot();
            CHECK(root->next1 == nullptr);
            CHECK(root->next0 != nullptr);
            CHECK(root->next0->next1 == nullptr);
            CHECK(root->next0->next0 == nullptr);
        }
    }

    SUBCASE("getTable") {
        CHECK_EQ(Table{{'A', std::vector<bool> {0}}}, tree2.getTable());
        CHECK_EQ(Table{{'a', std::vector<bool> {0}}, {'b', std::vector<bool>{1, 0}}, {'c', std::vector<bool>{1, 1}}},
                 tree1.getTable());
    }

    SUBCASE("go") {
        SUBCASE("case_1") {
            const Node* curNode1 = tree1.getRoot();
            CHECK(!curNode1->terminalFlag);
            curNode1 = tree1.go(curNode1, 0);
            CHECK(curNode1->terminalFlag);
            CHECK_EQ(curNode1->word, static_cast<uint8_t>('a'));
            CHECK_THROWS_AS(tree1.go(curNode1, 1), const HuffmanLogicError&);
        }
        SUBCASE("case_2") {
            const Node* curNode1 = tree1.getRoot();
            curNode1 = tree1.go(curNode1, 1);
            CHECK(!curNode1->terminalFlag);
            curNode1 = tree1.go(curNode1, 0);
            CHECK(curNode1->terminalFlag);
            CHECK_EQ(curNode1->word, static_cast<uint8_t>('b'));
        }
        SUBCASE("case_3") {
            const Node* curNode2 = tree2.getRoot();
            CHECK(!curNode2->terminalFlag);
            CHECK_THROWS_AS(tree2.go(curNode2, 1), const HuffmanLogicError&);
            curNode2 = tree2.go(curNode2, 0);
            CHECK(curNode2->terminalFlag);
            CHECK_EQ(curNode2->word, static_cast<uint8_t>('A'));
        }
    }
}

TEST_CASE("InputStream") {
    std::ifstream in(pathToResources("InputStreamSample.txt"));
    InputBitStream inputBitStream(in, "InputStreamSample.txt");
    for (char c = 'a'; c <= 'z'; c++) {
        for (int j = 0; j < 8; j++) {
            CHECK(inputBitStream.readBit() == ((c >> j) & 1));
        }
    }
    CHECK_THROWS_AS(inputBitStream.readBit(), const HuffmanInvalidCompressedFile&);
}

TEST_CASE("OutputStream") {
    std::ofstream out(pathToResources("OutputStream.txt"));
    OutputBitStream outputBitStream(out, "OutputStream.txt");
    for (char c = 'a'; c <= 'z'; c++) {
        std::vector<bool> toWrite(8, 0);
        for (int j = 0; j < 8; j++) {
            toWrite[j] = (c >> j) & 1;
        }
        outputBitStream.write(toWrite);
    }
    outputBitStream.close();
    out.close();
    CHECK(system(("diff " + pathToResources("InputStreamSample.txt") + " " + pathToResources("OutputStream.txt")).c_str()) == 0);
    system(("rm " + pathToResources("OutputStream.txt")).c_str());
}

TEST_CASE("code and decode simple") {
    codeAndDecodeCheck("sample_01.txt");
}

TEST_CASE("code and decode hard") {
    codeAndDecodeCheck("faust.txt", true, {5924106, 3736918, 844});
    codeAndDecodeCheck("img_1.png", true, {4795717, 4793166, 2320});
}

TEST_CASE("empty file") {
    codeAndDecodeCheck("empty.txt", true);
}

TEST_CASE("Table") {
    SUBCASE("default constructor") {
        Table table;
        for (std::size_t i = 0; i <= maxByte; i++) {
            CHECK(table[i].empty());
        }
    }

    SUBCASE("operator[]") {
        Table table;
        table[10] = std::vector<bool> {false, true, true, false};
        table[maxByte] = std::vector<bool> {true};
        table[0] = std::vector<bool> {false};
        CHECK_EQ(table[10], std::vector<bool> {false, true, true, false});
        CHECK_EQ(table[maxByte], std::vector<bool> {true});
        CHECK_EQ(table[0], std::vector<bool> {false});
        for (std::size_t i = 0; i <= maxByte; i++) {
            if (i == 0 || i == 10 || i == maxByte)
                continue;
            CHECK(table[i].empty());
        }
    }

    SUBCASE("constructor") {
        Table table{{0, std::vector<bool> {false}},
                    {72, std::vector<bool> {false, false, true}},
                    {maxByte, std::vector<bool> {true, false, true}}};
        CHECK_EQ(table[0], std::vector<bool> {false});
        CHECK_EQ(table[72], std::vector<bool> {false, false, true});
        CHECK_EQ(table[maxByte], std::vector<bool> {true, false, true});
        for (std::size_t i = 0; i <= maxByte; i++) {
            if (i == 0 || i == 72 || i == maxByte)
                continue;
            CHECK(table[i].empty());
        }

        Table table2{{1, std::vector<bool> {false}}};
        CHECK_EQ(table2[1], std::vector<bool> {false});
        for (std::size_t i = 0; i < maxByte; i++) {
            if (i == 1)
                continue;
            CHECK(table2[i].empty());
        }
    }

    SUBCASE("at") {
        Table table;
        CHECK_THROWS_AS(table.at(0), const HuffmanLogicError&);
        table[1] = std::vector<bool> (1, false);
        CHECK_NOTHROW(table.at(1));
        CHECK_THROWS_AS(table.at(maxByte), const HuffmanLogicError&);
        table[1].clear();
        CHECK_THROWS_AS(table.at(1), const HuffmanLogicError&);
    }
}