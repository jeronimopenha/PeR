//
// Created by jeronimo on 05/01/25.
//

#ifndef CACHE_H
#define CACHE_H

#include <vector>


class Cache {
private:
    int cacheLines;
    int cacheLinesExp;
    int cacheColumns;
    int cacheColumnsExp;
    std::vector<bool> cacheValid;
    std::vector<int> cacheTag;
    std::vector<std::vector<int> > cacheData;

public:
    Cache(int cacheLinesExp, int cacheColumnsExp);

    int readCache(int address, const std::vector<int> &vec);
};


#endif //CACHE_H
