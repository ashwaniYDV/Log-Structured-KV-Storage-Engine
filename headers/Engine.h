#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "FileOrchestrator.h"
#include "Record.h"

struct Engine {
    FileOrchestrator orchestrator;

    std::unordered_map<Cask, std::pair<std::string, size_t>, Cask::HashFunction> offsetMap; 

    Engine() {
        orchestrator.loadIndex(offsetMap);
    }

    ~Engine() {
        orchestrator.saveIndex(offsetMap);
    }

    void write(Cask &key, Cask &value)
    {
        Record record(key, value, PacketType::UPDATE);

        orchestrator.checkIfNewChunkNeeded(record.size());

        auto& activeStream = orchestrator.getActiveStream();

        size_t byteOffset = activeStream.tellp();

        offsetMap[record.key] = {orchestrator.getCurrentDataFilePath(), byteOffset };
        
        activeStream << record;
        activeStream.flush();
    }

    bool read(Cask &key, Record& record) {
        if (!offsetMap.count(key))
        {
            return false;
        }

        auto dataFilePath = offsetMap[key].first;
        std::ifstream file(dataFilePath, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Error opening " << dataFilePath << " for reading data." << std::endl;
            return false;
        }

        auto byteOffset = offsetMap[key].second;
        file.seekg(byteOffset);
        file >> record;

        return true;
    }

    void remove(Cask &key)
    {
        auto value = Cask();
        Record record(key, value, PacketType::DELETE);

        orchestrator.checkIfNewChunkNeeded(record.size());

        auto& activeStream = orchestrator.getActiveStream();

        size_t byteOffset = activeStream.tellp();

        offsetMap.erase(record.key);
        
        activeStream << record;
        activeStream.flush();
    }
};
