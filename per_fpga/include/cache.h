//
// Created by jeronimo on 05/01/25.
//

#ifndef CACHE_H
#define CACHE_H

#include <vector>

using namespace std;

class Cache {
private:
    int cacheLines;
    int cacheLinesExp;
    int cacheColumns;
    int cacheColumnsExp;
    vector<bool> cacheValid;
    vector<int> cacheTag;
    vector<vector<int> > cacheData;

public:
    Cache(int cacheLinesExp, int cacheColumnsExp);

    int readCache(int address, const vector<int> &vec);
};


#endif //CACHE_H
