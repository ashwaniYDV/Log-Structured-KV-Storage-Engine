#pragma once

#include <string>
#include <stdexcept>
#include <ostream>
#include <cstring>

using std::size_t;
using std::string;
using std::ostream;

struct Cask {
    char * data;
    size_t len;
    bool isSelfAllocated;
    
    Cask(): data(0), len(0), isSelfAllocated(false) {};

    Cask(int &datum) : isSelfAllocated(true) {
        len = sizeof(int);
        data = new char[len];
        memcpy(data, &datum, len);
    }

    Cask(double &datum): isSelfAllocated(true) {
        len = sizeof(double);
        data = new char[len];
        memcpy(data, &datum, len);
    }

    Cask(string &s): isSelfAllocated(true) {
        len = s.size();
        data = new char[len];
        memcpy(data, s.data(), len);
    }

    Cask(char *ptr, int len): data(ptr), len(len), isSelfAllocated(true) {}
    
    ~Cask() {
        delete data;
    }
    
    auto operator=(const Cask& other) -> void {
        this->len = other.len;
        this->data = new char[this->len];
        memcpy(this->data, other.data, this->len);
        this->isSelfAllocated = other.isSelfAllocated;
    }

    Cask(const Cask& other) {
        this->len = other.len;
        this->data = new char[this->len];
        memcpy(this->data, other.data, this->len);
        this->isSelfAllocated = other.isSelfAllocated;
    }

    auto ComputeHash() const -> size_t {
        size_t hashValue = 0;
        for(size_t i = 0; i < len; i++) {
            hashValue ^= static_cast<std::size_t>(data[i]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        }

        return hashValue;
    }

    struct HashFunction {
        std::size_t operator()(const Cask& cask) const {
            return cask.ComputeHash();
        }
    };

    auto size() const -> size_t {
        return len;
    }
    
    auto dataPtr() const  -> char* {
        return data;
    }

    auto operator[](size_t index) const -> char {
        if (index >= len) {
            throw std::runtime_error("Cannot index a Cask past the size");
        }

        return data[index];
    }

    auto operator==(const Cask &key) const -> bool {
        if (key.size() != this->len) return false;

        return !memcmp(key.dataPtr(), this->data, this->len);
    }

    friend auto operator<<(ostream &os, const Cask &cask) -> ostream& {
        // ISSUE: if string itself is of 4 bytes, then this line will parse it as an int
        // TODO: write a better parsing logic
        if (cask.len == sizeof(int)) {
            os << *(int*)(cask.data);
        } else if (cask.len == sizeof(double)) {
            os << *(double*)(cask.data);
        } else {
            os.write(cask.dataPtr(), cask.len);
        }

        return os;
    }

    template<typename T>
    auto convert(T &result) -> void {
        memcpy(&result, data, len);
    }
};