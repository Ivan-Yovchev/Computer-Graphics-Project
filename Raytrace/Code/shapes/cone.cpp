#include "cone.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

using namespace std;

#define EPSILON (1e-6)

Cone::CapHit Cone::getCapIntersection(const Vector &center, const Vector &normal, double r, const Ray &ray) {

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

Hit Cone::intersect(Ray const &ray)
{
    // Vectro from base center A
    // to cone top B
    Vector c = b - a;
    Vector normC = c.normalized();

    double r2c2 = r * r / c.length_2();
    Vector dConeTop = ray.O - b;
    double dotNormCD = normC.dot(ray.D);

    // The cone can be viwedas a cylinder with a linearly decreasing radius where at some point B r = 0
    // The previously described formula for the cylinder is modified to fit the decreasing radius.
    // ((p(t) - a) x (b - a))^2 = r^2 * ((b - a).(p(t) - a))^2 / (b - a)^2
    // some of the variables are multiplied by r2c2 due to the fact that the right side of the quation
    // is divided by (b - a)^2
    double x = ray.D.length_2() - r2c2 * (dotNormCD * dotNormCD) - dotNormCD * dotNormCD;
    double y = 2 * (ray.D.dot(dConeTop) - r2c2 * ray.D.dot(normC) * dConeTop.dot(normC) - ray.D.dot(normC) * dConeTop.dot(normC));
    double z = dConeTop.length_2() - r2c2 * dConeTop.dot(normC) * dConeTop.dot(normC) - dConeTop.dot(normC) * dConeTop.dot(normC);

    // Substituting in the quadratic formula to obtain possible number
    // of solutions
    double D = y * y - 4 * x * z;
    if(D < 0.0) {
        return Hit::NO_HIT();
    }

    // If D > 0 there are two solutions t
    // the shortest distance to the ray is needed
    // so only the solution t = (-b - sqrt(D)) / 2a
    // is calcualted
    double t = (-y - sqrt(D)) / (2 * x);

    // Point of intersection P
    Vector p = ray.O + t * ray.D;

    // Distance from point A to the
    // perpendicular from the intersecrion point P
    // to the main cone axis AB
    double alpha = (p - a).dot(normC);

    // Vector perpendicular to the main axis AB
    // to the point of intersection P
    // (distance between line and point in vector form)
    Vector q = p - (a + alpha * c.normalized());

    // distance alpha can be used to restrict
    // the height of the cone. This applies
    // only for the top of the cone
    if(alpha > c.length()) {
        return Hit::NO_HIT();
    }


    // The ray may still intersect with the bottom cap of the cone
    // the normal vector of the cap is the vector normC in opposite
    // direction
    if(alpha < 0.0) {
        CapHit h = getCapIntersection(a, -normC, r, ray);
        if(h.isHit) {
            Vector pBase = ray.O + h.t * ray.D;
            if((pBase - h.center).length() > r) {
                return Hit::NO_HIT();
            }
            return Hit(h.t, h.normal);
        }
        return Hit::NO_HIT();
    }

    // If the ray intersect with the body of the cone
    // the normal vector still needs to be calculated.
    // The Normal vector N is the same one for all intersection
    // points P of the same rotation of the cone. This means
    // N can be calculated based on the simpler case where
    // the perpendicular from P intersect A.

    // The intersection point between the side of the cone
    // and the base radius
    Point baseK = a + q.normalized() * r;

    // The side of the cone in vector form
    Vector k = b - baseK;

    // Angle between side of the cone and the base radius
    double theta = acos(r / k.length());

    // Length of the side PK where the intersection point
    // P is such that its perpendicular intersects with A
    double kside = r * sin((acos(-1) / 2) - theta);

    // The vector form of the side PK
    Vector ksideVector = kside * k.normalized();
    Point knormal = baseK + ksideVector;
    Vector N = (knormal - a).normalized();

    return Hit(t, N);
}

Cone::Cone(Point const &a, Point const &b, double r)
:
    a(a),
    b(b),
    r(r)
{}


Point Cone::mapTexture(Ray const &ray, Hit const &hit) {
    return Point{0, 0, 1};
}
