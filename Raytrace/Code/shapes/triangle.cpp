#include "triangle.h"

#include <cmath>
#include <iostream>

using namespace std;

#define EPSILON (1e-6)

Hit Triangle::intersect(Ray const &ray)
{
    Vector edge1 = v1 - v3;
    Vector edge2 = v2 - v3;

    // Möller–Trumbore intersection
    Vector h = ray.D.cross(edge2);
    double a = edge1.dot(h);
    if(-EPSILON < a && a < EPSILON) {
        return Hit::NO_HIT();
    }
    double f = 1 / a;
    Vector s = ray.O - v3;
    double u = f * (s.dot(h));
    if(u < 0.0 - EPSILON || u > 1.0 + EPSILON) {
        return Hit::NO_HIT();
    }
    Vector q = s.cross(edge1);
    double v = f * ray.D.dot(q);
    if(v < 0.0 - EPSILON || (u + v) > 1.0 + EPSILON) {
        return Hit::NO_HIT();
    }
    double t = f * edge2.dot(q);
    if(t < EPSILON) {
        return Hit::NO_HIT();
    }

    Vector N = u * n1 + v * n2 + (1 - u - v) * n3;

    return Hit(t, N);
}

Triangle::Triangle(Vertex const &v1, Vertex const &v2, Vertex const &v3)
:
    v1(Point(v1.x, v1.y, v1.z)),
    v2(Point(v2.x, v2.y, v2.z)),
    v3(Point(v3.x, v3.y, v3.z)),
    n1(Vector(v1.nx, v1.ny, v1.nz)),
    n2(Vector(v2.nx, v2.ny, v2.nz)),
    n3(Vector(v3.nx, v3.ny, v3.nz))
{}

Triangle::Triangle(Point const &v1, Point const &v2, Point const &v3)
:
    v1(Point(v1.x, v1.y, v1.z)),
    v2(Point(v2.x, v2.y, v2.z)),
    v3(Point(v3.x, v3.y, v3.z)),
    n1(((v2 - v1).cross(v3 - v1)).normalized()),
    n2(((v2 - v1).cross(v3 - v1)).normalized()),
    n3(((v2 - v1).cross(v3 - v1)).normalized())
{}

Point Triangle::mapTexture(Ray const &ray, Hit const &hit) {
    return Point{0, 0, 1};
}
