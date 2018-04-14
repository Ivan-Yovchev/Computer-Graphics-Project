#include "raytracer.h"

#include "image.h"
#include "light.h"
#include "material.h"
#include "triple.h"
#include "objloader.h"
#include "fs-utils.h"

// =============================================================================
// -- Include all your shapes here ---------------------------------------------
// =============================================================================

#include "shapes/sphere.h"
#include "shapes/triangle.h"
#include "shapes/cylinder.h"
#include "shapes/cone.h"

// =============================================================================
// -- End of shape includes ----------------------------------------------------
// =============================================================================

#include "json/json.h"

#include <exception>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace std;        // no std:: required
using json = nlohmann::json;

bool Raytracer::parseObjectNode(json const &node)
{
    ObjectPtr obj = nullptr;

// =============================================================================
// -- Determine type and parse object parametrers ------------------------------
// =============================================================================

    if (node["type"] == "sphere")
    {
        Point pos(node["position"]);
        double radius = node["radius"];
        if(node.find("rotation") != node.end() && node.find("angle") != node.end()) {
            Vector rotationAxis(node["rotation"]);
            double rotationAngle(fmod(static_cast<double>(node["angle"]) * M_PI / 180, 2 * M_PI));
            obj = ObjectPtr(new Sphere(pos, radius, rotationAngle, rotationAxis));
        } else {
            obj = ObjectPtr(new Sphere(pos, radius));
        }
    }
    else if(node["type"] == "triangle")
    {
        Point v1(node["v1"]);
        Point v2(node["v2"]);
        Point v3(node["v3"]);
        obj = ObjectPtr(new Triangle(v1, v2, v3));
    }
    else if(node["type"] == "cylinder")
    {
        Point a(node["a"]);
        Point b(node["b"]);
        double r = node["radius"];
        obj = ObjectPtr(new Cylinder(a, b, r));
    }
    else if(node["type"] == "cone")
    {
        Point a(node["a"]);
        Point b(node["b"]);
        double r = node["radius"];
        obj = ObjectPtr(new Cone(a, b, r));
    }
    else if (node["type"] == "mesh")
    {
        std::string filename = node["path"];
        try {
            OBJLoader loader(filename);
            std::vector<Face> faces = loader.face_data();
            std::cout << faces.size() << std::endl;
            for(Face face : faces) {
                if(face.vertices.size() != 3) {
                    std::stringstream msg;
                    msg << "Unsupported face type, expected 3 vertices per face, got " << face.vertices.size() << " vertices";
                    throw std::runtime_error(msg.str());
                }
                ObjectPtr obj = ObjectPtr(new Triangle(face.vertices.at(0), face.vertices.at(1), face.vertices.at(2)));
                obj->material = parseMaterialNode(node["material"]);
                scene.addObject(obj);
            }
        } catch(OBJLoader::Error e) {
            // The previous code did not throw error on failure to parse.
            // This code will show the exact line of the error within the *.obj file.
            cout << e.filename() << ":" << e.line() << ": Error parsing file!" << endl;
            return false;
        }

        return true;
    }
    else
    {
        cerr << "Unknown object type: " << node["type"] << ".\n";
    }

// =============================================================================
// -- End of object reading ----------------------------------------------------
// =============================================================================

    if (!obj)
        return false;

    // Parse material and add object to the scene
    obj->material = parseMaterialNode(node["material"]);
    scene.addObject(obj);
    return true;
}

Light Raytracer::parseLightNode(json const &node) const
{
    Point pos(node["position"]);
    Color col(node["color"]);
    return Light(pos, col);
}

Material Raytracer::parseMaterialNode(json const &node) const
{
    if(node.find("color") != node.end()) {
        Color color(node["color"]);
        double ka = node["ka"];
        double kd = node["kd"];
        double ks = node["ks"];
        double n  = node["n"];
        return Material(color, ka, kd, ks, n);
    }
    if(node.find("texture") != node.end() && node["texture"].is_string()) {
        double ka = node["ka"];
        double kd = node["kd"];
        double ks = node["ks"];
        double n  = node["n"];
        string relPath = node["texture"];
        return Material(dirname + '/' + relPath, ka, kd, ks, n);
    }
    throw runtime_error("Object must have either \"color\" or \"texture\" keys");
}

bool Raytracer::readScene(string const &ifname)
try
{
    // Read and parse input json file
    dirname = fs::dirname(ifname);
    ifstream infile(ifname);
    if (!infile) throw runtime_error("Could not open input file for reading.");
    json jsonscene;
    infile >> jsonscene;

// =============================================================================
// -- Read your scene data in this section -------------------------------------
// =============================================================================

    Point eye(jsonscene["Eye"]);
    scene.setEye(eye);

    if(jsonscene["Shadows"].is_boolean() && jsonscene["Shadows"]) {
        scene.shadows(true);
    }

    if(jsonscene["MaxRecursionDepth"].is_number()) {
        int value = jsonscene["MaxRecursionDepth"];
        if(value < 0) {
            value = 0;
        }
        scene.maxDepthRecursion(static_cast<unsigned>(value));
    }

    if(jsonscene["SuperSamplingFactor"].is_number()) {
        int value = jsonscene["SuperSamplingFactor"];
        if(value < 1) {
            value = 1;
        }
        scene.superSamplingFactor(value);
    }

    for (auto const &lightNode : jsonscene["Lights"])
        scene.addLight(parseLightNode(lightNode));

    unsigned objCount = 0;
    for (auto const &objectNode : jsonscene["Objects"])
        if (parseObjectNode(objectNode))
            ++objCount;

    cout << "Parsed " << objCount << " objects.\n";

// =============================================================================
// -- End of scene data reading ------------------------------------------------
// =============================================================================

    return true;
}
catch (exception const &ex)
{
    cerr << ex.what() << '\n';
    return false;
}

void Raytracer::renderToFile(string const &ofname)
{
    // TODO: the size may be a settings in your file
    Image img(400, 400);
    cout << "Tracing...\n";
    scene.render(img);
    cout << "Writing image to " << ofname << "...\n";
    img.write_png(ofname);
    cout << "Done.\n";
}
