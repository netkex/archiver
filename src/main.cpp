//
// Created by Daniil Pavlenko on 20/04/2021.
//

#include <iostream>
#include "Huffman.hpp"

using namespace huffman;

enum taskType {
    CODE,
    DECODE,
    UNDEFINED
};

struct Arguments {
    std::string inputFile;
    std::string outputFile;
    taskType type = UNDEFINED;
    bool timeFlag = false;
};

Arguments parse(int argc, char* argv[]) {
    Arguments result;
    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "-f" || arg == "--file") {
            if (i == argc - 1)
                throw std::invalid_argument("No file after -f flag");
            if (!result.inputFile.empty())
                throw std::invalid_argument("Too many -f flags");
            result.inputFile = std::string(argv[i + 1]);
            i += 1;
            continue;
        }
        if (arg == "-o" || arg == "--output") {
            if (i == argc - 1)
                throw std::invalid_argument("No file after -o flag");
            if (!result.outputFile.empty())
                throw std::invalid_argument("Too many -o flags");
            result.outputFile = std::string(argv[i + 1]);
            i += 1;
            continue;
        }
        if (arg == "-u" || arg == "-c") {
            if (result.type != UNDEFINED)
                throw std::invalid_argument("Too many tasks flags");
            result.type = (arg == "-u") ? DECODE : CODE;
            continue;
        }
        if (arg == "-t") {
            result.timeFlag = true;
            continue;
        }
        throw std::invalid_argument("No such flag " + arg);
    }
    if (result.inputFile.empty())
        throw std::invalid_argument("Input file wasn't stated");
    if (result.outputFile.empty())
        throw std::invalid_argument("Output file wasn't stated");
    if (result.type == UNDEFINED)
        throw std::invalid_argument("Task flag wasn't stated");
    return result;
}

int main(int argc,char* argv[]) {
    Arguments arguments;
    try {
        arguments = parse(argc, argv);
    } catch (const std::exception &e) {
        std::cout << "Error in arguments!" << std::endl << e.what() << std::endl;
        return -1;
    }

    try {
        auto startTime = clock();
        if (arguments.type == CODE) {
            auto statisticSize = code(arguments.inputFile, arguments.outputFile);
            std::cout << statisticSize.originalSize << std::endl << statisticSize.compressedSize << std::endl
                      << statisticSize.headerSize << std::endl;
        } else {
            auto statisticSize = decode(arguments.inputFile, arguments.outputFile);
            std::cout << statisticSize.compressedSize << std::endl << statisticSize.originalSize << std::endl
                      << statisticSize.headerSize << std::endl;
        }
        auto endTime = clock();
        if (arguments.timeFlag)
            std::cout << "Time of execution: " << (endTime - startTime) * 1. / CLOCKS_PER_SEC << " seconds"
                      << std::endl;
    } catch (const HuffmanException &e) {
        std::cout << "Huffman error!" << std::endl << e.what() << std::endl;
        return -1;
    } catch (const std::exception &e) {
        std::cout << "Runtime error!" << std::endl << e.what() << std::endl;
        return -1;
    }
    return 0;
}