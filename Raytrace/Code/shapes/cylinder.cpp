#include "cylinder.h"

#include <cmath>
#include <limits>
#include <iostream>

#define EPSILON (1e-6)

using namespace std;

Cylinder::CapHit Cylinder::getCapIntersection(const Vector &center, const Vector &normal, double r, const Ray &ray) {

    // Intersection with a plane
    double denom = normal.dot(ray.D);
    if(denom < -EPSILON) {
        double t = (center - ray.O).dot(normal) / denom;
        if(t < 0.0) {
            return CapHit{false, 0, Vector(), Vector()};
        }
        return CapHit{true, t, normal, center};
    }
    return CapHit{false, 0, Vector(), Vector()};
}


Hit Cylinder::intersect(Ray const &ray)
{
    // Vector going through both centers of the cylinder
    Vector c = b - a;

    // Deriving the formula for a cylinder
    // ((p(t) - a) x (b - a))^2 = r^2 * (b - a)^2
    // (ray.O + t*ray.D - a) x (b - a))^2 = r^2 * (b - a)^2
    // Hence:
    // x = (ray.O - a) x (b - a)
    // y = ray.D x (b - a)
    // z = r^2 * (b - a)^2
    // Substitute in t^2y^2 + 2*(x.y)t + x^2 - z = 0
    // tfor t1,2 solutions
    Vector x = (ray.O - a).cross(c);
    Vector y = ray.D.cross(c);
    double z = r * r * c.length_2();

    // Discriminant
    // D = b^2 - 4ac = 4(x.y)^2 - 4y^2(x^2 - z)
    double D = 4 * x.dot(y) * x.dot(y) - 4 * y.length_2() * (x.length_2() - z);

    if(D < 0.0) {
        return Hit::NO_HIT();
    }

    // If D > 0 there are two solutions t
    // the shortest distance to the ray is needed
    // so only the solution t = (-b - sqrt(D)) / 2a
    // is calcualted
    double t = (-2 * x.dot(y) - sqrt(D)) / (2 * y.length_2());

    // Point of intersection
    Point p = ray.O + t * ray.D;

    // Distance from point A to the
    // perpendicular from the intersecrion point P
    // to the main cylinder axis AB
    double alpha = (p - a).dot(c.normalized());

    // Vector perpendicular to the main axis AB
    // to the point of intersection P
    // (distance between line and point in vector form)
    Vector q = p - (a + alpha * c.normalized());

    // Normal vector is the same as vector Q
    // going oitside of the cylinder
    Vector N = q.normalized();

    // distance alpha can be used to restrict
    // the height of the cylinder
    if(alpha < 0.0 || alpha > c.length()) {

        // Ray might still intersect one of the two caps
        // of the cylinder (top and bottom)
        // The normal vector for the caps is the vector C
        // normalized. For the bottom cap its direction is reversed
        Vector n = c.normalized();
        CapHit bottom = getCapIntersection(a, -n, r, ray);
        CapHit top = getCapIntersection(b, n, r, ray);
        CapHit selected;

        if(top.isHit && bottom.isHit) {
            if(top.t < bottom.t) {
                selected = top;
            } else {
                selected = bottom;
            }
        } else if(bottom.isHit) {
            selected = bottom;
        } else if(top.isHit) {
            selected = top;
        } else {
            return Hit::NO_HIT();
        }

        // If the plane of circle is intersected
        // the caps have to be restricted by the radius
        Vector pBase = ray.O + selected.t * ray.D;
        if((pBase - selected.center).length() > r) {
            return Hit::NO_HIT();
        }
        return Hit(selected.t, selected.normal);
    }

    return Hit(t, N);
}

Cylinder::Cylinder(Point const &a, Point const &b, double r)
:
    a(a),
    b(b),
    r(r)
{}


Point Cylinder::mapTexture(Ray const &ray, Hit const &hit) {
    return Point{0, 0, 1};
}
