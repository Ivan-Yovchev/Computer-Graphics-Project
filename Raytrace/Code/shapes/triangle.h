#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "../object.h"
#include "../vertex.h"

class Triangle: public Object
{
    public:
        Triangle(Vertex const &v1, Vertex const &v2, Vertex const &v3);
        Triangle(Point const &v1, Point const &v2, Point const &v3);

        virtual Hit intersect(Ray const &ray);

        Point const v1;
        Point const v2;
        Point const v3;
        Vector const n1;
        Vector const n2;
        Vector const n3;

        Point mapTexture(Ray const &ray, Hit const &hit);
};

#endif
