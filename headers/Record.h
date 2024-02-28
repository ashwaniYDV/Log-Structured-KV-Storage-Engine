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

enum RecordType {
    UPDATE,
    DELETE
};

struct Record {
    time_t timestamp;
    size_t keySize;
    size_t valueSize;
    RecordType recordType;
    Cask key;
    Cask value;
    unsigned char checksum;

    // simple XOR checksum
    // TODO: use better cryptographic hash functions
    auto calculateChecksum() -> void {
        checksum = 0;

        checksum ^= static_cast<unsigned char>(timestamp);
        checksum ^= static_cast<unsigned char>(keySize);
        checksum ^= static_cast<unsigned char>(valueSize);
        checksum ^= static_cast<unsigned char>(recordType);

        const char* keyData = key.dataPtr();
        for (size_t i = 0; i < keySize; ++i)
            checksum ^= static_cast<unsigned char>(keyData[i]);

        const char* valueData = value.dataPtr();
        for (size_t i = 0; i < valueSize; ++i)
            checksum ^= static_cast<unsigned char>(valueData[i]);
    }

    Record(
        const time_t timestamp,
        const Cask &key,
        const Cask &value,
        const RecordType &rType)
        : timestamp(timestamp), keySize(key.size()), valueSize(value.size()), recordType(rType), key(key), value(value) {}

    Record(
        const Cask &key,
        const Cask &value,
        const RecordType &rType) 
        : keySize(key.size()), valueSize(value.size()), recordType(rType), key(key), value(value) {
        timestamp = system_clock::to_time_t(system_clock::now());
    }

    Record() {}
    
    // Record(const Record &other)
    // {
    //     *this = std::move(Record(other));
    // }

    Record(const Record &other)
        : timestamp(other.timestamp),
        keySize(other.keySize),
        valueSize(other.valueSize),
        recordType(other.recordType),
        key(other.key),
        value(other.value)
    {}


    friend auto operator<<(ofstream &fs, const Record &record) -> ofstream& {
        // Calculate and set the checksum on the original record
        const_cast<Record&>(record).calculateChecksum();

        fs.write(reinterpret_cast<const char *>(&record.timestamp), sizeof(record.timestamp));
        fs.write(reinterpret_cast<const char *>(&record.keySize), sizeof(record.keySize));
        fs.write(reinterpret_cast<const char *>(&record.valueSize), sizeof(record.valueSize));
        fs.write(reinterpret_cast<const char *>(&record.recordType), sizeof(record.recordType));
        fs.write(record.key.dataPtr(), record.keySize);
        fs.write(record.value.dataPtr(), record.valueSize);

        fs.write(reinterpret_cast<const char *>(&record.checksum), sizeof(record.checksum));
        
        return fs;
    }

    friend auto operator<<(ostream& os, const Record& record) -> ostream& {
        os << "{timestamp: " << record.timestamp << ", recordType: " << record.recordType << ", key: " << record.key << ", value: " << record.value << ", checksum: "
           << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(record.checksum) << "}" << std::endl;
        return os;
    }

    friend auto operator>>(istream &is, Record &record) -> istream& {
        is.read(reinterpret_cast<char *>(&record.timestamp), sizeof(record.timestamp));
        is.read(reinterpret_cast<char *>(&record.keySize), sizeof(record.keySize));
        is.read(reinterpret_cast<char *>(&record.valueSize), sizeof(record.valueSize));
        is.read(reinterpret_cast<char *>(&record.recordType), sizeof(record.recordType));

        Cask keyCask(new char[record.keySize], record.keySize);
        Cask valueCask(new char[record.valueSize], record.valueSize);
        is.read(keyCask.dataPtr(), record.keySize);
        is.read(valueCask.dataPtr(), record.valueSize);
        record.key = keyCask;
        record.value = valueCask;

        // Read checksum from the file
        is.read(reinterpret_cast<char *>(&record.checksum), sizeof(record.checksum));

        // Verify the checksum
        record.calculateChecksum();

        Record recordCopy(record);
        recordCopy.calculateChecksum();

        if (record.checksum != recordCopy.checksum) {
            std::cerr << "Checksum mismatch. Record is corrupted." << std::endl;
        }

        return is;
    }

    auto operator==(const Record &record) const -> bool {
        return (
            record.recordType == this->recordType and
            record.timestamp == this->timestamp and
            record.keySize == this->keySize and
            record.valueSize == this->valueSize and
            record.key == this->key and
            record.value == this->value);
    }

    auto size() const -> size_t {
        return sizeof(time_t) + sizeof(keySize) + sizeof(valueSize) + sizeof(RecordType) + keySize + valueSize;
    }
};
