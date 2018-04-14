#ifndef CYLINDER_H_
#define CYLINDER_H_

#include "../object.h"

class Cylinder: public Object
{
    // Struct CapHit and function getCapIntersection
    // are repeated in both the Cylinder and Cone .cpp and .h
    // files. A new class should be created which holds the
    // struct and function for shapes with caps, and Cylinder
    //and Cone should inherit that class. However, due to the
    // time constraints no such class was created.
    typedef struct {
        bool isHit;
        double t;
        Vector normal;
        Vector center;
    } CapHit;

    static CapHit getCapIntersection(const Vector &center, const Vector &normal, double r, const Ray &ray);
    public:
        Cylinder(Point const &a, Point const &b, double r);

        virtual Hit intersect(Ray const &ray);

        Point const a;
        Point const b;
        double r;

        Point mapTexture(Ray const &ray, Hit const &hit);
};

#endif
