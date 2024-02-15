#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>

#include "Engine.h"

class KeyValueStore
{
private:
    Engine engine;

public:
    KeyValueStore() : engine()
    {
    }

    template<typename Key, typename Value>
    void put(Key &key, Value &value)
    {
        Cask k(key);
        Cask v(value);
        engine.write(k, v);
    }

    template<typename Key>
    Record get(Key &key)
    {
        Cask k(key);
        Record record;
        if (!engine.read(k, record))
        {
            std::cerr << "Key: {" << key << "} not found." << std::endl;
        }
        return record;
    }

    template<typename Key>
    void remove(Key &key)
    {
        Cask k(key);
        engine.remove(k);
    }
};
