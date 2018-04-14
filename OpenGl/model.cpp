#include "model.h"

#include <set>
#include <cassert>
#include <QDebug>
#include <QFile>
#include <QTextStream>

Model::Model(QString filename, Flags flags) : flags(flags)
{
    hNorms = false;
    hTexs = false;

    qDebug() << ":: Loading model:" << filename;
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        QString line;
        QStringList tokens;

        while(!in.atEnd())
        {
            line = in.readLine().trimmed();
            if(line.length() <= 0)
            {
                continue;
            }

            if (line.startsWith("#")) continue; // skip comments

            tokens = line.split(" ", QString::SkipEmptyParts);

            // Switch depending on first element
            if (tokens[0] == "v")
            {
                parseVertex(tokens);
            }

            if (tokens[0] == "vn" )
            {
                parseNormal(tokens);
            }

            if (tokens[0] == "vt" )
            {
                parseTexture(tokens);
            }

            if (tokens[0] == "f" )
            {
                parseFace(tokens);
            }
        }

        file.close();

        if(!reqNorms)
        {
            // Normals might exists, but are not used in faces
            hNorms = false;
        }

        if(!reqTexs)
        {
            // TexCoords might exists, but are not used in faces
            hTexs = false;
        }

        if(flags & Flags::UNPACK)
        {
            // create an array version of the data
            unpackIndexes();
        }

        // Allign all vertex indices with the right normal/texturecoord indices
        alignData();

        if(flags & Flags::TANGENT)
        {
            calculateTangentSpace();
        }
    }
}

/**
 * @brief Model::unitze Not Implemented yet!
 *
 * Unitize the model by scaling so that it fits a box with sides 1
 * and origin at 0,0,0
 * Usefull for models with different scales
 *
 */
void Model::unitize()
{
    qDebug() << "TODO: implement this yourself";
}

std::vector<QVector3D> Model::getVertices()
{
    return vertices;
}

std::vector<QVector3D> Model::getNormals()
{
    return normals;
}

std::vector<QVector2D> Model::getTextureCoords()
{
    return textureCoords;
}

std::vector<QVector3D> Model::getVertices_indexed()
{
    return vertices_indexed;
}

std::vector<QVector3D> Model::getNormals_indexed()
{
    return normals_indexed;
}

std::vector<QVector2D> Model::getTextureCoords_indexed()
{
    return textureCoords_indexed;
}

std::vector<unsigned>  Model::getIndices()
{
    return indices;
}

std::vector<float> Model::getVNInterleaved() const
{
    std::vector<float> buffer;

    for (std::size_t i = 0; i != vertices.size(); ++i)
    {
        QVector3D vertex = vertices.at(i);
        QVector3D normal = normals.at(i);
        buffer.push_back(vertex.x());
        buffer.push_back(vertex.y());
        buffer.push_back(vertex.z());
        buffer.push_back(normal.x());
        buffer.push_back(normal.y());
        buffer.push_back(normal.z());
    }

    return buffer;
}

std::vector<float> Model::getVNTInterleaved() const
{
    std::vector<float> buffer;

    for (std::size_t i = 0; i != vertices.size(); ++i)
    {
        QVector3D vertex = vertices.at(i);
        QVector3D normal = normals.at(i);
        QVector2D uv = textureCoords.at(i);
        buffer.push_back(vertex.x());
        buffer.push_back(vertex.y());
        buffer.push_back(vertex.z());
        buffer.push_back(normal.x());
        buffer.push_back(normal.y());
        buffer.push_back(normal.z());
        buffer.push_back(uv.x());
        buffer.push_back(uv.y());
    }

    return buffer;
}

std::vector<float> Model::getVNInterleaved_indexed() const
{
    std::vector<float> buffer;

    for (std::size_t i = 0; i != vertices_indexed.size(); ++i)
    {
        QVector3D vertex = vertices_indexed.at(i);
        QVector3D normal = normals_indexed.at(i);
        buffer.push_back(vertex.x());
        buffer.push_back(vertex.y());
        buffer.push_back(vertex.z());
        buffer.push_back(normal.x());
        buffer.push_back(normal.y());
        buffer.push_back(normal.z());
    }

    return buffer;
}

std::vector<float> Model::getVNTInterleaved_indexed() const
{
    std::vector<float> buffer;

    for (std::size_t i = 0; i != vertices_indexed.size(); ++i)
    {
        QVector3D vertex = vertices_indexed.at(i);
        QVector3D normal = normals_indexed.at(i);
        QVector2D uv = textureCoords_indexed.at(i);
        buffer.push_back(vertex.x());
        buffer.push_back(vertex.y());
        buffer.push_back(vertex.z());
        buffer.push_back(normal.x());
        buffer.push_back(normal.y());
        buffer.push_back(normal.z());
        buffer.push_back(uv.x());
        buffer.push_back(uv.y());
    }

    return buffer;
}

std::vector<float> Model::getVNTTBInterleaved_indexed() const
{
    std::vector<float> buffer;

    for (std::size_t i = 0; i != vertices_indexed.size(); ++i)
    {
        QVector3D vertex = vertices_indexed.at(i);
        QVector3D normal = normals_indexed.at(i);
        QVector2D uv = textureCoords_indexed.at(i);
        QVector3D tangent;
        QVector3D bitangent;
        if(flags & Flags::TANGENT)
        {
            tangent = tangent_indexed.at(i);
            bitangent = bitangent_indexed.at(i);
        }
        buffer.push_back(vertex.x());
        buffer.push_back(vertex.y());
        buffer.push_back(vertex.z());
        buffer.push_back(normal.x());
        buffer.push_back(normal.y());
        buffer.push_back(normal.z());
        buffer.push_back(uv.x());
        buffer.push_back(uv.y());
        buffer.push_back(tangent.x());
        buffer.push_back(tangent.y());
        buffer.push_back(tangent.z());
        buffer.push_back(bitangent.x());
        buffer.push_back(bitangent.y());
        buffer.push_back(bitangent.z());
    }

    return buffer;
}


/**
 * @brief Model::getNumTriangles
 *
 * Gets the number of triangles in the model
 *
 * @return number of triangles
 */
int Model::getNumTriangles()
{
    return vertices.size()/3;
}

void Model::parseVertex(QStringList tokens)
{
    float x,y,z;
    x = tokens[1].toFloat();
    y = tokens[2].toFloat();
    z = tokens[3].toFloat();
    vertices_indexed.push_back(QVector3D(x,y,z));
}

void Model::parseNormal(QStringList tokens)
{
    hNorms = true;
    float x,y,z;
    x = tokens[1].toFloat();
    y = tokens[2].toFloat();
    z = tokens[3].toFloat();
    norm.push_back(QVector3D(x,y,z));
}

void Model::parseTexture(QStringList tokens)
{
    hTexs = true;
    float u,v;
    u = tokens[1].toFloat();
    v = tokens[2].toFloat();
    tex.push_back(QVector2D(u,v));
}

void Model::parseFace(QStringList tokens)
{
    QStringList elements;
    Face face;

    for( int i = 1; i != tokens.size(); ++i )
    {
        elements = tokens[i].split("/");

        VertexIndices vertexIndices;

        // -1 since .obj count from 1
        vertexIndices.coord = elements[0].toUInt()-1;

        if ( elements.size() > 1 && ! elements[1].isEmpty() )
        {
            vertexIndices.texCoord = elements[1].toUInt()-1;
            reqNorms = vertexIndices.hasTexCoords = true;
        }

        if (elements.size() > 2 && ! elements[2].isEmpty() )
        {
            vertexIndices.normal = elements[2].toUInt()-1;
            reqTexs = vertexIndices.hasNormal = true;
        }
        if(reqNorms && !vertexIndices.hasNormal)
        {
            throw std::runtime_error("Inconsistent *.obj state: some faces specify normal vectors, others do not.");
        }
        if(reqTexs && !vertexIndices.hasTexCoords)
        {
            throw std::runtime_error("Inconsistent *.obj state: some faces specify texture vectors, others do not.");
        }
        face.vertices.push_back(vertexIndices);
    }
    faces.push_back(face);
}


/**
 * @brief Model::alignData
 *
 * Make sure that the indices from the vertices align with those
 * of the normals and the texture coordinates, create extra vertices
 * if vertex has multiple normals or texturecoords
 */
void Model::alignData()
{
    std::vector<QVector3D> verts = std::vector<QVector3D>();
    verts.reserve(vertices_indexed.size());
    std::vector<QVector3D> norms = std::vector<QVector3D>();
    norms.reserve(vertices_indexed.size());
    std::vector<QVector2D> texcs = std::vector<QVector2D>();
    texcs.reserve(vertices_indexed.size());
    std::vector<Vertex> vs = std::vector<Vertex>();

    std::vector<unsigned> ind = std::vector<unsigned>();
    ind.reserve(indices.size());

    std::vector<Face> sfc;

    unsigned currentIndex = 0;

    for (Face& fc : faces)
    {
        Face rfc;
        for(VertexIndices& i : fc.vertices)
        {
            VertexIndices ridx;
            QVector3D v = vertices_indexed.at(i.coord);

            QVector3D n = QVector2D(0,0);
            if ( hNorms )
            {
                if(!i.hasNormal)
                {
                    std::runtime_error("Missing normal");
                }
                n = norm.at(i.normal);
            }

            QVector2D t = QVector2D(0,0);
            if ( hTexs )
            {
                if(!i.hasTexCoords)
                {
                    std::runtime_error("Missing texture coordinates");
                }
                t = tex.at(i.texCoord);
            }

            Vertex k = Vertex(v,n,t);
            std::vector<Vertex>::const_iterator ik = std::find(vs.begin(), vs.end(), k);
            if(ik != vs.end())
            {
                unsigned index = ik - vs.begin();
                ridx.coord = ridx.normal = ridx.texCoord = index;
                ridx.hasNormal = i.hasNormal;
                ridx.hasTexCoords = i.hasTexCoords;
                ind.push_back(index);
            }
            else
            {
                unsigned index = currentIndex;
                verts.push_back(v);
                norms.push_back(n);
                texcs.push_back(t);
                ind.push_back(currentIndex);
                vs.push_back(k);
                ridx.coord = ridx.normal = ridx.texCoord = index;
                ridx.hasNormal = i.hasNormal;
                ridx.hasTexCoords = i.hasTexCoords;
                currentIndex++;
            }
            rfc.vertices.push_back(ridx);
        }
        sfc.push_back(rfc);
    }

    // Remove old data
    vertices_indexed.clear();
    normals_indexed.clear();
    textureCoords_indexed.clear();
    indices.clear();
    faces.clear();

    // Set the new data
    vertices_indexed = verts;
    normals_indexed = norms;
    textureCoords_indexed = texcs;
    indices = ind;
    faces = sfc;
}

/**
 * @brief Model::unpackIndexes
 *
 * Unpack indices so that they are available for glDrawArrays()
 *
 */
void Model::unpackIndexes()
{
    vertices.clear();
    normals.clear();
    textureCoords.clear();

    for(Face& face : faces)
    {
        for(VertexIndices& idx : face.vertices)
        {
            vertices.push_back(vertices_indexed.at(idx.coord));
            if(hNorms)
            {
                if(!idx.hasNormal)
                {
                    throw std::runtime_error("Missing normal");
                }
                normals.push_back(norm.at(idx.normal));
            }
            if(hTexs)
            {
                if(!idx.hasTexCoords)
                {
                    throw std::runtime_error("Missing texture coordinates");
                }
                textureCoords.push_back(tex.at(idx.texCoord));
            }
        }
    }
}

void Model::calculateTangentSpace()
{
    std::vector<QVector3D> tg(indices.size());
    std::vector<QVector3D> btg(indices.size());
    std::set<unsigned> htg;
    if(!hTexs || !hNorms)
    {
        throw std::runtime_error("Unable to calculate tangent space, when no texture coordinates present");
    }
    unsigned currentIndex = 0;
    for(Face& face : faces)
    {
        for(std::size_t vtxIndex = 0; vtxIndex < face.vertices.size(); ++vtxIndex)
        {
            VertexIndices& vtx = face.vertices.at(vtxIndex);
            if(!vtx.hasTexCoords || !vtx.hasNormal)
            {
                throw std::runtime_error("Unable to calculate tangent space, when no texture coordinates or normals present");
            }
            if(htg.find(vtx.coord) != htg.end()) {
                continue;
            }
            std::array<VertexIndices, 3> vertices;
            vertices[0] = vtx;
            for(std::size_t i = 1; i < 3; ++i)
            {
                bool found = false;
                for(std::size_t vtxIndex2 = 0; vtxIndex2 < face.vertices.size(); ++vtxIndex2)
                {
                    VertexIndices& anotherVertex = face.vertices.at((vtxIndex + vtxIndex2 + 1) % face.vertices.size());
                    bool alreadyUsed = false;
                    for(std::size_t j = 0; j < i; ++j)
                    {
                        if(vertices.at(j) == anotherVertex)
                        {
                            alreadyUsed = true;
                        }
                    }
                    if(!alreadyUsed)
                    {
                        vertices[i] = anotherVertex;
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    std::runtime_error("Error during calculation of tangent space: cannot find 3 different vertices for a face");
                }
            }
            QVector3D normal = normals_indexed.at(vtx.normal);
            QVector3D v_a = vertices_indexed.at(vertices.at(0).coord);
            QVector3D v_b = vertices_indexed.at(vertices.at(1).coord);
            QVector3D v_c = vertices_indexed.at(vertices.at(2).coord);
            QVector2D t_a = textureCoords_indexed.at(vertices.at(0).texCoord);
            QVector2D t_b = textureCoords_indexed.at(vertices.at(1).texCoord);
            QVector2D t_c = textureCoords_indexed.at(vertices.at(2).texCoord);
            QVector3D v_ab = v_b - v_a;
            QVector3D v_ac = v_c - v_a;
//            float t_ab_x = t_b.x() - t_a.x();
            float t_ab_y = t_b.y() - t_a.y();
//            float t_ac_x = t_c.x() - t_a.x();
            float t_ac_y = t_c.y() - t_a.y();
            QVector3D tangent(
                t_ac_y * v_ab.x() - t_ab_y * v_ac.x(),
                t_ac_y * v_ab.y() - t_ab_y * v_ac.y(),
                t_ac_y * v_ab.z() - t_ab_y * v_ac.z()
            );
            QVector3D realBitangent = QVector3D::crossProduct(normal, tangent).normalized();
            QVector3D realTangent = QVector3D::crossProduct(realBitangent, normal).normalized();
            //qDebug() << "N.T" << QVector3D::dotProduct(normal, realTangent);
            //qDebug() << "N.B" << QVector3D::dotProduct(normal, realBitangent);
            //qDebug() << "T.B" << QVector3D::dotProduct(realTangent, realBitangent);

            tg[vtx.coord] = realTangent;
            btg[vtx.coord] = realBitangent;
            htg.insert(vtx.coord);
            currentIndex++;
        }
    }

    tangent_indexed.clear();
    bitangent_indexed.clear();

    tangent_indexed = tg;
    bitangent_indexed = btg;
}
