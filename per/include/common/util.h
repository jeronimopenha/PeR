#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <iostream>
#include <fstream>
//#include <string>
#include <vector>
#include <random>
#include <algorithm>


namespace fs = std::filesystem;


template<typename T>
void randomVector(std::vector<T> &vec) {
    std::random_device rd;
    std::mt19937 g(rd());
    shuffle(vec.begin(), vec.end(), g);
}

std::string getProjectRoot();

std::string verifyPath(const std::string &path);

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path,
    const std::string &file_extension
);

std::string funcKey(const std::string &a, const std::string &b);


long getManhattanDist(long cell1, long cell2, long n_cells_sqrt);


void createDir(const fs::path &pth);

long getX(long cellIndex, long nCellsSqrt);

long getY(long cellIndex, long nCellsSqrt);

long getCellIndex(long x, long y, long nCellsSqrt);

int randomInt(int min, int max);

float randomFloat(float min, float max);

#endif
