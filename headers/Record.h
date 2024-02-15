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

enum PacketType
{
    UPDATE,
    DELETE
};

// Overload << operator for PacketType enum
// std::ostream& operator<<(std::ostream& os, PacketType c) {
//     switch (c) {
//         case UPDATE:
//             os << "UPDATE"; 
//             break;
//         case DELETE: 
//             os << "DELETE"; 
//             break;
//         default: 
//             os << ""; 
//             break;
//     }
//     return os;
// }

struct Record
{
    time_t timestamp;
    size_t keySize;
    size_t valueSize;
    PacketType packetType;
    Cask key;
    Cask value;

    Record(
        const time_t timestamp,
        const Cask &key,
        const Cask &value,
        const PacketType &pType)
        : timestamp(timestamp), keySize(key.size()), valueSize(value.size()), packetType(pType), key(key), value(value) {}

    Record(
        const Cask &key,
        const Cask &value,
        const PacketType &pType) : keySize(key.size()), valueSize(value.size()), packetType(pType), key(key), value(value)
    {
        timestamp = system_clock::to_time_t(system_clock::now());
    }

    Record() {
        
    }
    
    Record(const Record &other)
    {
        *this = std::move(Record(other));
    }

    friend auto operator<<(ofstream &fs, const Record &record) -> ofstream &
    {
        fs.write(reinterpret_cast<const char *>(&record.timestamp), sizeof(record.timestamp));
        fs.write(reinterpret_cast<const char *>(&record.keySize), sizeof(record.keySize));
        fs.write(reinterpret_cast<const char *>(&record.valueSize), sizeof(record.valueSize));
        fs.write(reinterpret_cast<const char *>(&record.packetType), sizeof(record.packetType));
        fs.write(record.key.dataPtr(), record.keySize);
        fs.write(record.value.dataPtr(), record.valueSize);
        
        return fs;
    }

    friend auto operator<<(ostream &os, const Record &record) -> ostream &
    {
        os << "{timestamp: " << record.timestamp << ", packetType: " << record.packetType << ", key: " << record.key << ", value: " << record.value << "}" << std::endl;
        return os;
    }

    friend auto operator>>(istream &is, Record &record) -> istream &
    {
        is.read(reinterpret_cast<char *>(&record.timestamp), sizeof(record.timestamp));
        is.read(reinterpret_cast<char *>(&record.keySize), sizeof(record.keySize));
        is.read(reinterpret_cast<char *>(&record.valueSize), sizeof(record.valueSize));
        is.read(reinterpret_cast<char *>(&record.packetType), sizeof(record.packetType));

        Cask keyCask(new char[record.keySize], record.keySize);
        Cask valueCask(new char[record.valueSize], record.valueSize);
        is.read(keyCask.dataPtr(), record.keySize);
        is.read(valueCask.dataPtr(), record.valueSize);
        record.key = keyCask;
        record.value = valueCask;

        return is;
    }

    auto operator==(const Record &record) const -> bool
    {
        return (
            record.packetType == this->packetType and
            record.timestamp == this->timestamp and
            record.keySize == this->keySize and
            record.valueSize == this->valueSize and
            record.key == this->key and
            record.value == this->value);
    }

    auto size() const -> size_t
    {
        return sizeof(time_t) + sizeof(keySize) + sizeof(valueSize) + sizeof(PacketType) + keySize + valueSize;
    }
};
