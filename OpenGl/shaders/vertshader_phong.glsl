#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout (location = 0) in vec3 vertCoordinates_in;
layout (location = 1) in vec3 vertNormal_in;
layout (location = 2) in vec2 texCoordinates_in;
layout (location = 3) in vec3 tangent_in;
layout (location = 4) in vec3 bitangent_in;

// Specify the Uniforms of the vertex shader
uniform mat4 modelTransform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;
uniform mat3 normalModelTransform;

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

// Specify the output of the vertex stage
out vec3 worldPosition;
out vec3 modelNormalVector;
out vec3 modelTangentVector;
out vec3 modelBitangentVector;
out vec2 textureCoordinates;

void main()
{
    // gl_Position is the output (a vec4) of the vertex shader
    gl_Position = projectionTransform * (viewTransform * (modelTransform * vec4(vertCoordinates_in, 1.0)));

    worldPosition = vec3(modelTransform * vec4(vertCoordinates_in, 1.0));
    modelNormalVector = vertNormal_in;
    modelTangentVector = tangent_in;
    modelBitangentVector = bitangent_in;
    textureCoordinates = texCoordinates_in;
}
