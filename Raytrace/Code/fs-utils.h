#ifndef FS_UTILS_H_
#define FS_UTILS_H_

#include <string>

namespace fs {
    std::string realpath(std::string path);
    std::string dirname(std::string path);
}

#endif //FS_UTILS_H_
