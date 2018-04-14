#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QStringList>
#include <QVector2D>
#include <QVector3D>

#include <array>
#include <map>
#include <vector>

/**
 * @brief The Model class
 *
 * Loads all data from a Wavefront .obj file
 * IMPORTANT! Current only supports TRIANGLE meshes!
 *
 * Support for other meshes can be implemented by students
 *
 */
class Model
{
    // A Private Vertex class for vertex comparison
    // DO NOT include "vertex.h" or something similar in this file
    struct Vertex
    {

        QVector3D coord;
        QVector3D normal;
        QVector2D texCoord;

        Vertex() : coord(), normal(), texCoord() {}
        Vertex(QVector3D coords, QVector3D normal, QVector3D texc): coord(coords), normal(normal), texCoord(texc) {}

        bool operator==(const Vertex &other) const
        {
            if (other.coord != coord)
                return false;
            if (other.normal != normal)
                return false;
            if (other.texCoord != texCoord)
                return false;
            return true;
        }
    };

    struct VertexIndices
    {
        unsigned coord;
        unsigned normal;
        unsigned texCoord;
        bool hasNormal;
        bool hasTexCoords;

        bool operator==(const VertexIndices &other) const
        {
            if (other.coord != coord)
                return false;
            if (other.normal != normal)
                return false;
            if (other.texCoord != texCoord)
                return false;
            return true;
        }
    };

    struct Face {
        std::vector<VertexIndices> vertices;
    };

public:
    enum class Flags : unsigned
    {
        NONE = 0x0,
        UNPACK = 0x1,
        TANGENT = 0x2
    };
public:
    Model(QString filename, Flags flags = Flags::NONE);
    // It is okay, because all data is move-able.
    Model(Model&&) = default;

    // Used for glDrawArrays()
    std::vector<QVector3D> getVertices();
    std::vector<QVector3D> getNormals();
    std::vector<QVector2D> getTextureCoords();

    // Used for interleaving into one buffer for glDrawArrays()
    std::vector<float> getVNInterleaved() const;
    std::vector<float> getVNTInterleaved() const;

    // Used for glDrawElements()
    std::vector<QVector3D> getVertices_indexed();
    std::vector<QVector3D> getNormals_indexed();
    std::vector<QVector2D> getTextureCoords_indexed();
    std::vector<unsigned>  getIndices();

    // Used for interleaving into one buffer for glDrawElements()
    std::vector<float> getVNInterleaved_indexed() const;
    std::vector<float> getVNTInterleaved_indexed() const;
    std::vector<float> getVNTTBInterleaved_indexed() const;

    bool hasNormals();
    bool hasTextureCoords();
    int getNumTriangles();

    void unitize();

private:

    // OBJ parsing
    void parseVertex(QStringList tokens);
    void parseNormal(QStringList tokens);
    void parseTexture(QStringList tokens);
    void parseFace(QStringList tokens);
    void calculateTangentSpace();

    // Alignment of data
    void alignData();
    void unpackIndexes();

    // Intermediate storage of values
    std::vector<QVector3D> vertices_indexed;
    std::vector<QVector3D> normals_indexed;
    std::vector<QVector2D> textureCoords_indexed;
    std::vector<QVector3D> tangent_indexed;
    std::vector<QVector3D> bitangent_indexed;
    std::vector<unsigned> indices;

    std::vector<Face> faces;
    std::vector<QVector3D> vertices;
    std::vector<QVector3D> normals;
    std::vector<QVector2D> textureCoords;

    // Utility storage
    std::vector<QVector3D> norm;
    std::vector<QVector2D> tex;

    bool hNorms;
    bool hTexs;

    bool reqNorms;
    bool reqTexs;

    Flags flags;
};

inline Model::Flags operator|(const Model::Flags& left, const Model::Flags& right) {
    return static_cast<Model::Flags>(static_cast<std::underlying_type<Model::Flags>::type>(left) | static_cast<std::underlying_type<Model::Flags>::type>(right));
}

inline bool operator&(const Model::Flags& left, const Model::Flags& right) {
    return static_cast<std::underlying_type<Model::Flags>::type>(left) & static_cast<std::underlying_type<Model::Flags>::type>(right);
}

#endif // MODEL_H
