#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the input locations of attributes
layout (location = 0) in vec3 vertCoordinates_in;
layout (location = 1) in vec3 vertNormal_in;
layout (location = 2) in vec2 texCoordinates_in;

// Specify the Uniforms of the vertex shader
 uniform mat4 modelTransform;
 uniform mat4 viewTransform;
 uniform mat4 projectionTransform;
 uniform mat3 normalModelTransform;
 uniform mat3 normalViewTransform;

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
out float diffuseFactor;
out float specularFactor;
out vec2 textureCoordinates;

void main()
{
    gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vertCoordinates_in, 1.0);

    vec3 worldPosition = vec3(modelTransform * vec4(vertCoordinates_in, 1.0));
    vec3 worldNormalVector = normalize(normalModelTransform * vertNormal_in);
    vec3 worldLightVector = normalize(worldPosition - light.position);
    vec3 worldReflectedLightVector = normalize(reflect(worldLightVector, worldNormalVector));
    diffuseFactor = max(dot(-worldLightVector, worldNormalVector), 0.0f);

    vec3 viewPosition = vec3(viewTransform * vec4(worldPosition, 1.0));
    vec3 viewLightPosition = vec3(viewTransform * vec4(light.position, 1.0));
    vec3 viewLightVector = normalize(viewPosition - viewLightPosition);
    vec3 viewNormalVector = normalViewTransform * worldNormalVector;
    vec3 viewReflectedLightVector = normalize(reflect(viewLightVector, viewNormalVector));
    vec3 viewObserverVector = normalize(-viewPosition);
    specularFactor = pow(max(dot(viewReflectedLightVector, viewObserverVector), 0.0f), material.specularExponent);

    textureCoordinates = texCoordinates_in;
}
