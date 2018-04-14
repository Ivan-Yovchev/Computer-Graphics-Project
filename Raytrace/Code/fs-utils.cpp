#include "fs-utils.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <numeric>
#include <sstream>
#include <vector>

std::string fs::realpath(std::string path) {
    char *c_result = ::realpath(path.c_str(), nullptr);
    if (c_result == nullptr) {
        std::ostringstream msg;
        msg << "realpath() failed [" << ::strerror(errno) << "] for path: " << path;
        throw std::runtime_error(msg.str());
    }
    std::string result(c_result);
    free(c_result);
    return result;
}

std::string fs::dirname(std::string path) {
    std::istringstream stream(::fs::realpath(path));
    std::string buffer;
    std::vector<std::string> dirs;
    while (std::getline(stream, buffer, '/')) {
        dirs.push_back(buffer);
    }
    if (dirs.size() > 1) {
        dirs.pop_back();
    }
    std::ostringstream result;
    for(auto& dir : dirs) {
        result << dir << '/';
    }
    buffer = result.str();
    buffer.pop_back();
    return buffer;
}