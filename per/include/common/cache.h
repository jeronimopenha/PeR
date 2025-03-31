#ifndef CACHE_H
#define CACHE_H

#include <common/parameters.h>
#include <vector>

using namespace std;

class Cache {
private:
    vector<bool> cacheValid;
    vector<int> cacheTag;
    vector<vector<int> > cacheData;

public:
    Cache();

    int readCache(int address, const vector<int> &vec);
};


#endif
