#ifndef OBJLOADER_H_
#define OBJLOADER_H_

// Pro C++ Tip: only include header of types/classes you use in your
// header file (.h), other headers you need should go in your source
// file (.cpp / .cc)

#include "face.h"
#include "vertex.h"

#include <string>
#include <vector>
#include <exception>

class OBJLoader
{
    bool d_hasTexCoords;

    struct vec3
    {
        float x;
        float y;
        float z;
    };

    struct vec2
    {
        float u;
        float v;
    };

    std::vector<vec3> d_coordinates;
    std::vector<vec3> d_normals;
    std::vector<vec2> d_texCoords;

    /**
     * @brief The Vertex struct
     * Contains indices into the above
     * QVectors to be able to reconstruct
     * the model
     */
    struct Vertex_idx
    {
        size_t d_coord;
        size_t d_norm;
        size_t d_tex;
    };

    std::vector<Vertex_idx> d_vertices;
    std::vector<std::vector<size_t>> d_faces;
    unsigned d_current_line;

    typedef std::vector<std::string> StringList;

    public:
        class Error;

        /**
         * @brief OBJLoader
         * @param filename
         */
        explicit OBJLoader(std::string const &filename);

        /**
         * @brief vertex_data
         * @return interleaved vertex data, see vertex.h
         *
         * @note texCoord is only valid when hasTexCoords() returns
         *  true
         */
        std::vector<Vertex> vertex_data() const;

        /**
         * @brief face_data
         */
        std::vector<Face> face_data() const;

        unsigned numTriangles() const;

        bool hasTexCoords() const;

        /**
         * @brief unitize: scale mesh to fit in unitcube
         *
         * TODO: Implement this method yourself!
         */
        void unitize();

    private:

        void parseFile(std::string const &filename);
        void parseLine(std::string const &line);
        void parseVertex(StringList const &tokens);
        void parseNormal(StringList const &tokens);
        void parseTexCoord(StringList const &tokens);
        void parseFace(StringList const &tokens);

        StringList split(std::string const &str,
                             char splitChar,
                             bool keepEmpty = true);

};

class OBJLoader::Error {
    std::string m_filename;
    unsigned m_line;
    std::exception_ptr m_exception;

public:
    Error(std::string filename, unsigned, std::exception_ptr);
    virtual ~Error();
    std::string filename() const;
    unsigned line() const;
    const std::exception_ptr& exception() const;
};

#endif // OBJLOADER_H_
