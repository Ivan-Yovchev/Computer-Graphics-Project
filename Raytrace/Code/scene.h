#ifndef SCENE_H_
#define SCENE_H_

#include "light.h"
#include "object.h"
#include "triple.h"
#include "hit.h"

#include <utility>
#include <vector>

// Forward declerations
class Ray;
class Image;

class Scene
{
    std::vector<ObjectPtr> objects;
    std::vector<LightPtr> lights;   // no ptr needed, but kept for consistency
    Point eye;
    bool m_shadows;
    unsigned m_max_depth_recursion;
    unsigned m_super_sampling_factor;

protected:
    std::pair<ObjectPtr, Hit> traceToObject(Ray const &ray);
    Color phongIllumination(const Ray& ray, const std::pair<ObjectPtr, Hit> &intersection, const Material& material, const Light &source, unsigned flags);
    Color getMaterialColor(const Ray& ray, const Material& material, const std::pair<ObjectPtr, Hit>& intersection);

public:
    Scene();

    // trace a ray into the scene and return the color
    Color trace(Ray const &ray, unsigned depth = 0);

    // render the scene to the given image
    void render(Image &img);


    void addObject(ObjectPtr obj);
    void addLight(Light const &light);
    void setEye(Triple const &position);

    unsigned getNumObject();
    unsigned getNumLights();

    bool shadows() const;
    void shadows(bool);

    unsigned maxDepthRecursion() const;
    void maxDepthRecursion(unsigned);

    unsigned superSamplingFactor() const;
    void superSamplingFactor(unsigned);
};

#endif
