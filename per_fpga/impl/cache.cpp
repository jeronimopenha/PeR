#include "cache.h"

Cache::Cache(const int cacheLinesExp, const int cacheColumnsExp) {
    this->cacheLinesExp = cacheLinesExp;
    this->cacheLines = 1 << this->cacheLinesExp;

    this->cacheColumnsExp = cacheColumnsExp;
    this->cacheColumns = 1 << cacheColumnsExp;

    this->cacheValid = std::vector(cacheLines, false);
    this->cacheTag = std::vector<int>(cacheLines);
    this->cacheData = std::vector(cacheLines, std::vector<int>(cacheColumns));
}

int Cache::readCache(const int address, const std::vector<int> &vec) {
    const int tag = address >> (cacheLinesExp + cacheColumnsExp);
    int column = address & (cacheColumns - 1);
    const int line = (address >> cacheColumnsExp) & (cacheLines - 1);

    const bool cacheMiss = !cacheValid[line] | (cacheTag[line] != tag);

    if (cacheMiss) {
        const int readAddressoffset = (address >> cacheColumnsExp) << cacheColumnsExp;
        const int readTag = readAddressoffset >> (cacheLinesExp + cacheColumnsExp);
        cacheTag[line] = readTag;
        cacheValid[line] = true;

        for (int i = 0; i < cacheColumns; i++) {
            int readAddress = readAddressoffset + i;
            if (readAddress < vec.size())
                cacheData[line][i] = vec[readAddressoffset + i];
            else
                cacheData[line][i] = 0;
        }
    }

    return (cacheMiss) ? 1 : 0;
}
