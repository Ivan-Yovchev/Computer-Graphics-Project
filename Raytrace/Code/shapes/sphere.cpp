#include "sphere.h"
#include "../debug.h"

#include <cmath>
#include <iostream>

using namespace std;

Hit Sphere::intersect(Ray const &ray)
{
    // Vector from ray origin to center of sphere
    Vector OC = (ray.O - center);

    double a = ray.D.length_2();
    double b = 2 * OC.dot(ray.D);
    double c = OC.length_2() - radius * radius;

    double D = b * b - 4 * a * c;

    if(D < 0.0) {
        return Hit::NO_HIT();
    }
    double t1 = (-b + sqrt(D)) / (2 * a);
    double t2 = (-b - sqrt(D)) / (2 * a);

    double t = std::min(t1, t2);
    if(t < 0.0) {
        t = std::max(t1, t2);
        if(t < sqrt(std::numeric_limits<decltype(t)>::epsilon())) {
            return Hit::NO_HIT();
        }
    }

    Vector N = ray.O + t * ray.D - center;
    N.normalize();

    return Hit(t,N);
}

Sphere::Sphere(Point const &center, double radius, double rotationAngle, Vector rotationAxis)
:
    center(center),
    radius(radius),
    rotationAxis(rotationAxis.normalized()),
    rotationAngle(rotationAngle)
{}


Point Sphere::mapTexture(Ray const &ray, Hit const &hit) {
    // Texture map is made from rectangular image (highly deformed).
    // The line at y = 0 and y = height - 1 of the image correspond to north and south poles (as points)
    // So the closer to the poles we get, mode deformed the texture is.

    // Texture X is defined between coordinates 0.0 and 1.0. It wraps around after these coordinates.
    // Texture Y is defined between coordinates 0.0 and 1.0. It clamps to these coordinate.

    // Texture is always cut at vector X (1, 0, 0), initially. Then a sphere is rotated around an axis.
    // So we need to know the new vector of rotation.
    //Vector rotationAxis(0.0, 0.0, 1.0);
    //double rotationAngle = 0.0;

    Vector X(1, 0, 0);

    Vector N_2D = Vector(hit.N.x, hit.N.y, 0.0);
    N_2D.normalize();

    // Rodrigues rotation formula
    Vector X_rotated = X * cos(rotationAngle) + rotationAxis.cross(X) * sin(rotationAngle) + rotationAxis * rotationAxis.dot(X) * (1 - cos(rotationAngle));

    // Now we look at rotationAxis as a normal vector to a plane.
    // U coordinates are mapped too the circle formed on that plane.

    // Remainder: Projection over plane is: Nx(SxN), where N is the normal vector for the plane, S is the source vector.
    Vector U_N_projection = rotationAxis.cross(hit.N.cross(rotationAxis)).normalized();

    // X vector however is not on that plane. But the U coordinates are the same for all latitudes,
    // so we can safely project it onto the plane.
    Vector U_X_axis = rotationAxis.cross(X_rotated.cross(rotationAxis)).normalized();
    Vector U_Y_axis = rotationAxis.cross(U_X_axis);

    // Now both vectors are onto the plane and have length 1. We can create trigonometric circumference and measure
    // the angle between them. We could measure the sine and cosine and to the atan(sin/cos) to find it.
    // Sine is given by length of the cross product vector, cosine is given by dot product.
    double U_X_N_radians = atan2(U_Y_axis.dot(U_N_projection), U_X_axis.dot(U_N_projection));

    // Now we have in ranges:
    // [-pi; pi] / pi = [-1; 1]
    // [-1; 1] / 2 = [-0.5; 0.5]
    // [-0.5; 0.5] + 0.5 = [0.0; 1.0] = U coordinates
    double u = fmod(1.0 + U_X_N_radians / M_PI / 2.0, 1.0);

    // V coordinates are based solely between the difference of the angle between rotationAxis and the normal.
    // If they point in the same direction, dot is 1, acos(1) is 0
    // If they point in opposite direction, dot is -1, acos(-1) is pi
    // V is directly mapped in that range [0; pi]
    double rad_N_R_V = acos(rotationAxis.dot(hit.N));

    // Now we have in ranges:
    // [0; pi] / pi = [0.0; 1.0]
    double v = rad_N_R_V / M_PI;

    return Point{u, v, 0};
}
