#include "cache.h"

Cache::Cache() {
    this->cacheValid = std::vector(CACHE_LINES, false);
    this->cacheTag = std::vector<int>(CACHE_LINES);
    this->cacheData = vector(CACHE_LINES, vector<int>(CACHE_COLUMNS));
}

int Cache::readCache(const int address, const vector<int> &vec) {
    const int tag = address >> (CACHE_LINES_EXP + CACHE_COLUMNS_EXP);
    int column = address & (CACHE_COLUMNS - 1);
    const int line = (address >> CACHE_LINES_EXP) & (CACHE_LINES - 1);

    const bool cacheMiss = !cacheValid[line] | (cacheTag[line] != tag);

    if (cacheMiss) {
        const int readAddressoffset = (address >> CACHE_COLUMNS_EXP) << CACHE_COLUMNS_EXP;
        const int readTag = readAddressoffset >> (CACHE_LINES_EXP + CACHE_COLUMNS_EXP);
        cacheTag[line] = readTag;
        cacheValid[line] = true;

        for (int i = 0; i < CACHE_COLUMNS; i++) {
            int readAddress = readAddressoffset + i;
            if (readAddress < vec.size())
                cacheData[line][i] = vec[readAddressoffset + i];
            else
                cacheData[line][i] = 0;
        }
    }

    return (cacheMiss) ? 1 : 0;
}
