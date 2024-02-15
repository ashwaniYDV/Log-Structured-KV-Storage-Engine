#pragma once

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

#include "Cask.h"

using std::istream;
using std::ofstream;
using std::ostream;
using std::size_t;
using std::time_t;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::system_clock;

struct IndexKeyData
{
    size_t keySize;
    Cask key;
    size_t fileNameSize;
    std::string fileName;
    size_t offSet;

    IndexKeyData(
        const Cask &key,
        const std::string &fileName,
        const size_t offSet)
        : keySize(key.size()), fileNameSize(fileName.size()), key(key), fileName(fileName), offSet(offSet) {}

    IndexKeyData()
    {
    }

    IndexKeyData(const IndexKeyData &other)
    {
        *this = std::move(IndexKeyData(other));
    }

    friend auto operator<<(ofstream &fs, const IndexKeyData &ikd) -> ofstream &
    {
        fs.write(reinterpret_cast<const char *>(&ikd.keySize), sizeof(ikd.keySize));
        fs.write(reinterpret_cast<const char *>(&ikd.fileNameSize), sizeof(ikd.fileNameSize));
        fs.write(reinterpret_cast<const char *>(&ikd.offSet), sizeof(ikd.offSet));

        fs.write(ikd.key.dataPtr(), ikd.keySize);
        fs.write(ikd.fileName.c_str(), ikd.fileNameSize);

        return fs;
    }

    friend auto operator<<(ostream &os, const IndexKeyData &ikd) -> ostream &
    {
        os << "{key: " << ikd.key << ", fileName: " << ikd.fileName << ", offSet: " << ikd.offSet << "}" << std::endl;
        return os;
    }

    friend auto operator>>(istream &is, IndexKeyData &ikd) -> istream &
    {
        is.read(reinterpret_cast<char *>(&ikd.keySize), sizeof(ikd.keySize));
        is.read(reinterpret_cast<char *>(&ikd.fileNameSize), sizeof(ikd.fileNameSize));
        is.read(reinterpret_cast<char *>(&ikd.offSet), sizeof(ikd.offSet));

        Cask keyCask(new char[ikd.keySize], ikd.keySize);
        is.read(keyCask.dataPtr(), ikd.keySize);
        ikd.key = keyCask;

        std::string fileName(ikd.fileNameSize, '\0');
        is.read(&fileName[0], ikd.fileNameSize);
        ikd.fileName = fileName;

        return is;
    }

    auto operator==(const IndexKeyData &ikd) const -> bool
    {
        return (
            ikd.offSet == this->offSet and
            ikd.keySize == this->keySize and
            ikd.fileNameSize == this->fileNameSize and
            ikd.key == this->key and
            ikd.fileName == this->fileName);
    }

    auto size() const -> size_t
    {
        return sizeof(keySize) + sizeof(fileNameSize) + sizeof(offSet) + keySize + fileNameSize;
    }
};
