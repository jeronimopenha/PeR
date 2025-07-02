#include "common/cache.h"
#include <common/parametersFpga.h>

using namespace std;

Cache::Cache() {
    this->cacheValid = std::vector(CACHE_LINES, false);
    this->cacheTag = std::vector<long>(CACHE_LINES);
    this->cacheData = vector(CACHE_LINES, vector<long>(CACHE_COLUMNS));
}

long Cache::readCache(const long address, const vector<long> &vec) {
    const long tag = address >> (CACHE_LINES_EXP + CACHE_COLUMNS_EXP);
    //int column = address & (CACHE_COLUMNS - 1);
    const long line = (address >> CACHE_COLUMNS_EXP) & (CACHE_LINES - 1);

    const bool cacheMiss = !cacheValid[line] | (cacheTag[line] != tag);

    if (cacheMiss) {
        const long readAddressoffset = (address >> CACHE_COLUMNS_EXP) << CACHE_COLUMNS_EXP;
        const long readTag = readAddressoffset >> (CACHE_LINES_EXP + CACHE_COLUMNS_EXP);
        cacheTag[line] = readTag;
        cacheValid[line] = true;

        for (long i = 0; i < CACHE_COLUMNS; i++) {
            const long readAddress = readAddressoffset + i;
            if (readAddress < vec.size())
                cacheData[line][i] = vec[readAddressoffset + i];
            else
                cacheData[line][i] = 0;
        }
    }

    return (cacheMiss) ? 1 : 0;
}
