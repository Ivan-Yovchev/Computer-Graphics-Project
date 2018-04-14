#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColor;

struct Light {
    vec3 position;
    vec3 color;
};

struct Material {
    float emissionFactor;
    float ambientFactor;
    float diffuseFactor;
    float specularFactor;
    float specularExponent;
};

uniform Light light;
uniform Material material;

uniform sampler2D texImage;

in float diffuseFactor;
in float specularFactor;
in vec2 textureCoordinates;

void main()
{
    vec4 textureColor = texture2D(texImage, textureCoordinates);
    vec3 finalColor
            = material.emissionFactor * textureColor.rgb
            + material.ambientFactor * textureColor.rgb
            + material.diffuseFactor * diffuseFactor * light.color * textureColor.rgb
            + material.specularFactor * specularFactor * light.color;
    fColor = vec4(finalColor, textureColor.a);
}
