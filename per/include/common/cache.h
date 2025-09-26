#ifndef CACHE_H
#define CACHE_H
#include <vector>

#include <common/definitions.h>
#include <common/cachePar.h>

#ifdef USE_CACHE


class Cache {
    std::vector<bool> cacheValid;
    std::vector<long> cacheTag;
    std::vector<std::vector<long> > cacheData;

public:
    Cache();

    long readCache(long address, const std::vector<long> &vec);
};
#endif

#endif
