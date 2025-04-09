#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <iostream>
#include <fstream>
//#include <string>
#include <vector>
#include <random>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;


template<typename T>
void randomVector(vector<T> &vec) {
    random_device rd;
    mt19937 g(rd());
    shuffle(vec.begin(), vec.end(), g);
}

string getProjectRoot();

string verifyPath(const string &path);

vector<pair<string, string> > getFilesListByExtension(
    const string &path,
    const string &file_extension
);

string funcKey(const string &a, const string &b);


void saveToDot(const vector<pair<int, int> > &edges, const string &filename);


int getManhattanDist(int cell1, int cell2, int n_cells_sqrt);


void createDir(const fs::path &pth);

int getX(int cellIndex, int nCellsSqrt);

int getY(int cellIndex, int nCellsSqrt);

int getCellIndex(int x, int y, int nCellsSqrt);

int randomInt(int min, int max);

float randomFloat(float min, float max);

#endif
