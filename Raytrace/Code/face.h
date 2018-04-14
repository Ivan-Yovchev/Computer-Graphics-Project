#ifndef FACE_H_
#define FACE_H_

#include "vertex.h"

#include <vector>

struct Face {
    std::vector<Vertex> vertices;
};

#endif
