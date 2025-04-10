#ifndef CACHE_H
#define CACHE_H


#include <vector>

using namespace std;

class Cache
{
    vector<bool> cacheValid;
    vector<int> cacheTag;
    vector<vector<int>> cacheData;

public:
    Cache();

    int readCache(int address, const vector<int>& vec);
};


#endif
