#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include "IndexKeyData.h"

struct FileOrchestrator
{
    std::string rootFolder = "bin";
    std::string dataFolder = "bin/data/";
    std::string indexFolder = "bin/index/";

    size_t THRESHOLD_FILE_SIZE = 1000;

    std::ofstream activeStream;

    // vector to store all filenames in sorted order (latest file in index 0)
    std::vector<std::string> dataFiles;

    std::string getTimestamp() {
        // Get the current time point in milliseconds
        auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        std::string timestampString = std::to_string(currentTime);
        return timestampString;
    }

    std::string getCurrentDataFilePath() {
        return dataFolder + dataFiles[0];
    }

    std::string getIndexFilePath() {
        return indexFolder + "index";
    }

    int createFile(const std::string &filePath) {
        // Open the file for writing
        std::ofstream file(filePath);

        if (!file.is_open()) {
            std::cerr << "Error creating " << filePath << std::endl;
            return 1; // Return an error code
        }

        // Close the file
        file.close();

        return 0;
    }

    int createFolder(const std::string &folderPath) {
        if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath)) {
            // Create the folder if it does not exist
            if (std::filesystem::create_directory(folderPath)) {
                std::cout << folderPath << " created successfully" << std::endl;
            } else {
                std::cerr << "Failed to create " << folderPath << std::endl;
                return 1; // Return an error code
            }
        }
        return 0;
    }

    int initializeFilesAndFolders() {
        createFolder(rootFolder);
        createFolder(dataFolder);
        createFolder(indexFolder);

        if (std::filesystem::is_empty(dataFolder)) {
            std::string timestamp = getTimestamp();
            createFile(dataFolder + timestamp);
        }
        // if (std::filesystem::is_empty(indexFolder)) {
        //     createFile(getIndexFilePath());
        // }

        return 0;
    }

    void createNewChunk() {
        std::string timestamp = getTimestamp();
        createFile(dataFolder + timestamp);

        // add new file name to the front of globalChunks
        dataFiles.insert(dataFiles.begin(), timestamp);

        activeStream.close(); // later it will be opened by write() function with this new file
    }

    void createNewActiveStream() {
        std::string dataFilePath = getCurrentDataFilePath();
        activeStream = std::ofstream(dataFilePath, std::ios::ate | std::ios::app | std::ios::binary);
    }

    void loadAllFiles() {
        for (const auto & entry : std::filesystem::directory_iterator(dataFolder)) {
            dataFiles.push_back(entry.path().filename().string());
        }

        std::sort(dataFiles.begin(), dataFiles.end(), std::greater<std::string>());
    }

    FileOrchestrator()
    {
        initializeFilesAndFolders();
        loadAllFiles();
    }

    auto getActiveStream() -> std::ofstream& {
        if (!activeStream.is_open()) {
            createNewActiveStream();
            // if (!activeStream.is_open())
            // {
            //     std::cerr << "Error opening " << getCurrentDataFilePath() << " for writing data." << std::endl;
            //     return nullptr;
            // }
        }
        return activeStream;
    }

    void saveIndex(std::unordered_map<Cask, std::pair<std::string, size_t>, Cask::HashFunction>& offsetMap) {
        std::ofstream file(getIndexFilePath(), std::ios::binary | std::ios::trunc);
        if (!file.is_open())
        {
            std::cerr << "Error opening "<< getIndexFilePath() << " for persisting index." << std::endl;
            return;
        }

        // Write the key-value pairs to the index file in binary format
        for (const auto &pair : offsetMap)
        {
            IndexKeyData ikd(pair.first, pair.second.first, pair.second.second);
            file << ikd;
            file.flush();
        }

        file.close();

        std::cout << "Index saved to " << getIndexFilePath() << std::endl;
    }

    void loadIndex(std::unordered_map<Cask, std::pair<std::string, size_t>, Cask::HashFunction>& offsetMap)
    {
        std::ifstream file(getIndexFilePath(), std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Error opening " << getIndexFilePath() << " for loading index." << std::endl;
            return;
        }

        std::cout << "Loading these indexded" << std::endl;
        IndexKeyData ikd;
        while (file >> ikd) {
            std::cout << ikd;
            offsetMap[ikd.key] = {ikd.fileName, ikd.offSet};
        }

        file.close();

        std::cout << "Index loaded from " << getIndexFilePath() << std::endl;
    }

    auto getActiveStreamSize() -> size_t {
        if (activeStream.is_open()) {
            size_t byteOffset = activeStream.tellp();
            return activeStream.tellp();
        }
        return 0;
    }

    void checkIfNewChunkNeeded(size_t recordSize) {
        if (getActiveStreamSize() + recordSize > THRESHOLD_FILE_SIZE) {
            createNewChunk();
        }
    }
};
