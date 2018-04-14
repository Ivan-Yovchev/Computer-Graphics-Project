#ifndef SPHERE_H_
#define SPHERE_H_

#include "../object.h"

class Sphere: public Object
{
    public:
        Sphere(Point const &center, double radius, double rotationAngle = 0.0, Vector rotationAxis = Vector(0.0, 0.0, 1.0));

        virtual Hit intersect(Ray const &ray);

        Point const center;
        double const radius;
        Vector rotationAxis;
        double rotationAngle;

        Point mapTexture(Ray const &ray, Hit const &hit);
};

#endif
