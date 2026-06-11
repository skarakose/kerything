#ifndef KERYTHING_BTRFSSCANNERENGINE_H
#define KERYTHING_BTRFSSCANNERENGINE_H

#include <string>
#include <optional>
#include <vector>
#include <functional>
#include <unordered_map>
#include "../ScannerEngine.h"

namespace BtrfsScannerEngine {

    using ProgressCallback = std::function<void(uint64_t done, uint64_t total)>;

    struct BtrfsDatabase {
        std::vector<ScannerEngine::FileRecord> records;
        std::vector<char> stringPool;

        // Map from absolute paths (or inode numbers) to parentRecordIdx
        std::unordered_map<std::string, uint32_t> pathToIdx;

        void add(std::string_view name, const std::string& fullPath, const std::string& parentPath, uint64_t size, uint64_t modTime, bool isDir, bool isSymlink);
    };

    std::optional<BtrfsDatabase> scanDirectory(const std::string& mountPoint, ProgressCallback progressCb = {});

}

#endif //KERYTHING_BTRFSSCANNERENGINE_H
