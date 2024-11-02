//
// Created by jeronimo on 01/11/24.
//

#ifndef UTIL_H
#define UTIL_H

#include <filesystem>
#include <string>
#include <vector>
#include <utility>

std::string getProjectRoot();

std::string verifyPath(const std::string &path);

std::vector<std::pair<std::string, std::string> > getFilesListByExtension(
    const std::string &path, const std::string &file_extension);

#endif //UTIL_H
