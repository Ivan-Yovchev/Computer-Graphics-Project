#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColor;

uniform mat4 viewTransform;
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
uniform bool useNormalMap;

uniform sampler2D texImage;
uniform sampler2D normalImage;

// Current 3D position in world coordinates
in vec3 worldPosition;
// Interpolated normal vector in world coordinates
in vec3 modelNormalVector;
// Interpolated texture coordinates
in vec2 textureCoordinates;

in vec3 modelTangentVector;
in vec3 modelBitangentVector;

void main()
{
    vec3 tangentNormalOffset = texture2D(normalImage, textureCoordinates).rgb;
    mat3 tangentTransform = mat3(modelTangentVector, modelBitangentVector, modelNormalVector);
    vec3 tangentTextureNormal = normalize(texture2D(normalImage, textureCoordinates).xyz * 2.0 - 1.0);
    // The default normal map is dummy 1pixel image cotaning color that results in normal (0, 0, 1) (in tangent space)
    // This transformed to normal space (using T, B, N matrix, which contains the normal) result in the original normal
    vec3 modelTextureNormal = tangentTransform * tangentTextureNormal;

    // For the diffuse color we can use the model coordinates:
    // The light.position is already in world coordinates, otherwise it would be slower to convert only that to model coordinates
    // (Calculating the inverse of a matrix is neither fast, nor stable, e.g. there exists matrices that are not convertible)
    // The modelNormalVector is transfered to worldNormalVector by normalModelTransform, not by the modelTransform
    vec3 worldNormalVector = normalize(normalModelTransform * modelTextureNormal);
    vec3 worldLightVector = normalize(worldPosition - light.position);
    vec3 worldReflectedLightVector = normalize(reflect(worldLightVector, worldNormalVector));
    float diffuseFactor = max(dot(-worldLightVector, worldNormalVector), 0.0f);

    // Unfortunately, the specular is more complex, because the view vector is not available in world coordinates.
    // Instead we transform everything to view coordinates, but!
    // !!!Transforming the normal into view coordinates yields an incorrect vector.
    // Instead we need to extract normalTransform matrix from the viewTransform matrix (which is done by the CPU).
    vec3 viewPosition = vec3(viewTransform * vec4(worldPosition, 1.0));
    vec3 viewLightPosition = vec3(viewTransform * vec4(light.position, 1.0));
    vec3 viewLightVector = normalize(viewPosition - viewLightPosition);
    vec3 viewNormalVector = normalViewTransform * worldNormalVector;
    vec3 viewReflectedLightVector = normalize(reflect(viewLightVector, viewNormalVector));
    vec3 viewObserverVector = normalize(-viewPosition);
    float specularFactor = pow(max(dot(viewReflectedLightVector, viewObserverVector), 0.0f), material.specularExponent);

    vec4 textureColor = texture2D(texImage, textureCoordinates);

    vec3 resultColor
            = material.emissionFactor * textureColor.rgb // Emition of light
            + material.ambientFactor * textureColor.rgb // Ambient illumination
            + material.diffuseFactor * diffuseFactor * light.color * textureColor.rgb // Diffuse illumination
            + material.specularFactor * specularFactor * light.color; // Specular illumination
    fColor = vec4(resultColor, textureColor.a);
    //fColor = vec4(0.5 + modelTextureNormal / 2.0, 1.0);
}
