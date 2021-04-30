//
// Created by Daniil Pavlenko on 25/04/2021.
//

#ifndef HW_02_HUFFMANEXCEPTION_HPP
#define HW_02_HUFFMANEXCEPTION_HPP

namespace huffman {

class HuffmanException : public std::exception {
private:
    std::string message;
public:
    explicit HuffmanException(std::string message_ = "") : message(std::move(message_)) {}

    const char *what() const throw() override {
        return message.c_str();
    }
};

class HuffmanLoadFileException :  public HuffmanException {
public:
    explicit HuffmanLoadFileException(std::string file) : HuffmanException("Load file \"" + file + "\" error") {}
};

class HuffmanWriteFileException : public HuffmanException {
public:
    explicit HuffmanWriteFileException(std::string file) : HuffmanException("Write file \"" + file + "\" error") {}
};

class HuffmanInvalidCompressedFile : public HuffmanException {
public:
    explicit HuffmanInvalidCompressedFile(std::string file) : HuffmanException("Compressed file \"" + file + "\" is incorrect") {}
};

class HuffmanLogicError : public HuffmanException {
public:
    HuffmanLogicError() = default;
};

}

#endif //HW_02_HUFFMANEXCEPTION_HPP
