#include "scene.h"

#include "image.h"
#include "material.h"
#include "ray.h"
#include "debug.h"

#include <cmath>
#include <limits>
#include <algorithm>
#include <iostream>

using namespace std;

Color Scene::trace(Ray const &ray, unsigned depth)
{
    if (depth > m_max_depth_recursion) {
        return Color();
    }

    // Find hit object and distance
    auto objIntersecion = traceToObject(ray);

    // No hit? Return background color.
    if (!objIntersecion.first) return Color();

    Color ambient = objIntersecion.first->material.ka * getMaterialColor(ray, objIntersecion.first->material, objIntersecion);
    Color phong;

    for(auto light : lights) {
        phong += phongIllumination(ray, objIntersecion, objIntersecion.first->material, *light, 0x3);
    }

    if (depth + 1 <= m_max_depth_recursion) {
        Point intersectionPoint = ray.at(objIntersecion.second.t);
        Vector V = -ray.D;
        Vector R = 2 * (objIntersecion.second.N.dot(V)) * objIntersecion.second.N - V;
        R.normalize();
        Ray reflectRay(intersectionPoint, R);
        phong += objIntersecion.first->material.ks * trace(reflectRay, depth + 1);
    }

    return (ambient + phong);
}

Color Scene::phongIllumination(const Ray& ray, const std::pair<ObjectPtr, Hit>& intersection, const Material& material, const Light& light, unsigned flags) {
    Point intersectionPoint = ray.at(intersection.second.t);
    Vector L = light.position - intersectionPoint;
    L.normalize();
    Vector R = 2 * (intersection.second.N.dot(L)) * intersection.second.N - L;
    Vector V = -ray.D;
    Color phong;

    if(m_shadows) {
        Ray objectLightRay(intersectionPoint, (light.position - intersectionPoint).normalized());
        auto lightIntersection = traceToObject(objectLightRay);
        if(lightIntersection.first != nullptr && lightIntersection.first != intersection.first) {
            return Color();
        }
    }

    if(flags & 0x1) {
        double diffuse = std::max(0.0, L.dot(intersection.second.N));
        phong += material.kd * diffuse * light.color * getMaterialColor(ray, material, intersection);
    }
    if(flags & 0x2) {
        double specular = pow(std::max(0.0, R.dot(V)), material.n);
        phong += material.ks * light.color * specular;
    }

    return phong;
}

Color Scene::getMaterialColor(const Ray& ray, const Material& material, const pair<ObjectPtr, Hit>& intersection) {
    if(material.texture) {
        Point texCoords = intersection.first->mapTexture(ray, intersection.second);
        return material.texture->colorAt(texCoords.x, texCoords.y);
    }
    return material.color;
}

std::pair<ObjectPtr, Hit> Scene::traceToObject(const Ray& ray) {
    Hit min_hit(numeric_limits<double>::infinity(), Vector());
    ObjectPtr obj = nullptr;
    for (unsigned idx = 0; idx != objects.size(); ++idx)
    {
        Hit hit(objects[idx]->intersect(ray));
        if (hit.t < min_hit.t)
        {
            min_hit = hit;
            obj = objects[idx];
        }
    }
    return std::make_pair(obj, min_hit);
}

void Scene::render(Image &img)
{
    unsigned w = img.width();
    unsigned h = img.height();
    for (unsigned y = 0; y < h; ++y)
    {
        for (unsigned x = 0; x < w; ++x)
        {
            currentX = x;
            currentY = y;
            Color color;
            // Implementation of supersampling:
            // Pixel is a square with size 1x1
            // We divide the pixel to N*N squares with size (1/n)x(1/n)
            // For each square we take the center;
            // If supersampling is 1 (default), there is only one square and the middle was at it was before at (0.5)x(0.5)
            for(unsigned sy = 0; sy < m_super_sampling_factor; ++sy) {
                for(unsigned sx = 0; sx < m_super_sampling_factor; ++sx) {
                    double left = x + static_cast<double>(sx) / m_super_sampling_factor;
                    double right = x + static_cast<double>(sx + 1) / m_super_sampling_factor;
                    double top = (h - 1 - y) + static_cast<double>(sy) / m_super_sampling_factor;
                    double bottom = (h - 1 - y) + static_cast<double>(sy + 1) / m_super_sampling_factor;
                    double tx = (right + left) / 2;
                    double ty = (top + bottom) / 2;
                    Point pixel(tx, ty, 0);
                    Ray ray(eye, (pixel - eye).normalized());
                    Color subColor = trace(ray);
                    subColor.clamp();
                    color += subColor;
                }
            }
            color /= m_super_sampling_factor * m_super_sampling_factor;
            color.clamp();
            img(x, y) = color;
        }
    }
}

// --- Misc functions ----------------------------------------------------------

void Scene::addObject(ObjectPtr obj)
{
    objects.push_back(obj);
}

void Scene::addLight(Light const &light)
{
    lights.push_back(LightPtr(new Light(light)));
}

void Scene::setEye(Triple const &position)
{
    eye = position;
}

unsigned Scene::getNumObject()
{
    return objects.size();
}

unsigned Scene::getNumLights()
{
    return lights.size();
}

bool Scene::shadows() const {
    return m_shadows;
}

void Scene::shadows(bool shadows) {
    m_shadows = shadows;
}

unsigned Scene::maxDepthRecursion() const {
    return m_max_depth_recursion;
}

void Scene::maxDepthRecursion(unsigned value) {
    m_max_depth_recursion = value;
}

unsigned Scene::superSamplingFactor() const {
    return m_super_sampling_factor;
}

void Scene::superSamplingFactor(unsigned value) {
    m_super_sampling_factor = value;
}

Scene::Scene() : m_shadows{false}, m_max_depth_recursion{0}, m_super_sampling_factor{1} {
}
