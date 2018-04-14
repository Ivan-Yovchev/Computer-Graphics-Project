#ifndef GLMODELDATA_H
#define GLMODELDATA_H

#include <QOpenGLFunctions_3_3_Core>

class GLModelData
{
public:
    GLuint vbo, vao, ibo;
    GLsizei elementCount;
};

#endif // GLMODELDATA_H
