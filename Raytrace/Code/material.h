#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "triple.h"
#include "image.h"

#include <memory>
#include <string>

class Material
{
    public:
        Color color;        // base color
        std::unique_ptr<Image> texture;
        double ka;          // ambient intensity
        double kd;          // diffuse intensity
        double ks;          // specular intensity
        double n;           // exponent for specular highlight size

        Material() = default;

        Material(Color const &color, double ka, double kd, double ks, double n)
        :
            color(color),
            ka(ka),
            kd(kd),
            ks(ks),
            n(n)
        {}
        Material(std::string texturePath, double ka, double kd, double ks, double n) :
            texture(std::make_unique<Image>(texturePath)),
            ka(ka),
            kd(kd),
            ks(ks),
            n(n)
        {}
};

#endif
