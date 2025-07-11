#ifndef CACHE_H
#define CACHE_H


#include <vector>


class Cache {
    std::vector<bool> cacheValid;
    std::vector<long> cacheTag;
    std::vector<std::vector<long> > cacheData;

public:
    Cache();

    long readCache(long address, const std::vector<long> &vec);
};


#endif
