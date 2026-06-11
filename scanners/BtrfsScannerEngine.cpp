#include "BtrfsScannerEngine.h"
#include <filesystem>
#include <iostream>
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace BtrfsScannerEngine {

    void BtrfsDatabase::add(std::string_view name, const std::string& fullPath, const std::string& parentPath, uint64_t size, uint64_t modTime, bool isDir, bool isSymlink) {
        uint32_t parentIdx = 0xFFFFFFFF;
        
        if (!parentPath.empty()) {
            auto it = pathToIdx.find(parentPath);
            if (it != pathToIdx.end()) {
                parentIdx = it->second;
            }
        }

        ScannerEngine::FileRecord rec{};
        rec.parentRecordIdx = parentIdx;
        rec.size = size;
        rec.modificationTime = modTime;
        rec.isDir = isDir ? 1 : 0;
        rec.isSymlink = isSymlink ? 1 : 0;
        rec.reserved = 0;

        uint32_t offset = stringPool.size();
        rec.nameOffset = offset;
        rec.nameLen = static_cast<uint16_t>(name.size());

        stringPool.insert(stringPool.end(), name.begin(), name.end());
        
        uint32_t myIdx = records.size();
        records.push_back(rec);
        
        if (isDir) {
            pathToIdx[fullPath] = myIdx;
        }
    }

    std::optional<BtrfsDatabase> scanDirectory(const std::string& mountPoint, ProgressCallback progressCb) {
        BtrfsDatabase db;
        
        try {
            // Add root directory
            db.add("", mountPoint, "", 0, 0, true, false);

            uint64_t count = 0;
            auto options = fs::directory_options::skip_permission_denied;
            
            struct stat rootSt;
            if (lstat(mountPoint.c_str(), &rootSt) != 0) {
                return std::nullopt;
            }
            dev_t rootDev = rootSt.st_dev;

            std::error_code ec;
            fs::recursive_directory_iterator it(mountPoint, options, ec);
            fs::recursive_directory_iterator end;

            while (it != end && !ec) {
                const auto& entry = *it;
                
                std::string fullPath = entry.path().string();
                std::string parentPath = entry.path().parent_path().string();
                std::string name = entry.path().filename().string();
                
                bool isDir = entry.is_directory(ec);
                bool isSymlink = entry.is_symlink(ec);
                uint64_t size = 0;
                if (!isDir && !isSymlink) {
                    size = entry.file_size(ec);
                    if (ec) { size = 0; ec.clear(); }
                }
                
                uint64_t modTime = 0;
                struct stat st;
                if (lstat(fullPath.c_str(), &st) == 0) {
                    modTime = st.st_mtime;
                    if (isDir && st.st_dev != rootDev) {
                        it.disable_recursion_pending();
                    }
                }

                db.add(name, fullPath, parentPath, size, modTime, isDir, isSymlink);
                
                count++;
                if (progressCb && (count % 10000 == 0)) {
                    progressCb(count, count); // No total count known beforehand
                }
                
                it.increment(ec);
            }
            
            return db;
        } catch (const std::exception& e) {
            std::cerr << "BTRFS scan error: " << e.what() << "\n";
            return std::nullopt;
        }
    }

}
