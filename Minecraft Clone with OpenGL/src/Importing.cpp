#include "Importing.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool OBJModel::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "OBJModel::load — failed to open " << path << "\n";
        return false;
    }

    positions_.clear();
    normals_.clear();
    posIndices_.clear();
    normIndices_.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 p;
            iss >> p.x >> p.y >> p.z;
            positions_.push_back(p);
        }
        else if (prefix == "vn") {
            glm::vec3 n;
            iss >> n.x >> n.y >> n.z;
            normals_.push_back(n);
        }
        else if (prefix == "f") {
            // expecting "f v1//n1 v2//n2 v3//n3"
            for (int i = 0; i < 3; ++i) {
                std::string tok;
                iss >> tok;

                size_t firstSlash = tok.find('/');
                size_t secondSlash = tok.find('/', firstSlash + 1);

                if (firstSlash == std::string::npos || secondSlash == std::string::npos) {
                    std::cerr << "OBJModel::load — invalid face format: " << tok << "\n";
                    return false;
                }

                try {
                    unsigned pi = static_cast<unsigned>(std::stoi(tok.substr(0, firstSlash))) - 1;
                    // skip texture coordinate: tok.substr(firstSlash + 1, secondSlash - firstSlash - 1)
                    unsigned ni = static_cast<unsigned>(std::stoi(tok.substr(secondSlash + 1))) - 1;

                    posIndices_.push_back(pi);
                    normIndices_.push_back(ni);
                }
                catch (const std::exception& e) {
                    std::cerr << "OBJModel::load — invalid index: " << tok << " (" << e.what() << ")\n";
                    return false;
                }
            }
        }
        // ignore vt, o, g, usemtl, etc.
    }

    return true;
}

std::unique_ptr<Object> OBJModel::buildObject() const {
    if (positions_.empty() || normals_.empty() ||
        posIndices_.empty() || normIndices_.empty())
    {
        return nullptr;
    }

    // Zip into Vertex array + index buffer
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    verts.reserve(posIndices_.size());
    indices.reserve(posIndices_.size());

    for (size_t i = 0; i < posIndices_.size(); ++i) {
        Vertex v;
        v.position = positions_[posIndices_[i]];
        v.normal = normals_[normIndices_[i]];
        verts.push_back(v);
        indices.push_back(static_cast<unsigned int>(i));
    }

    return std::make_unique<Object>(verts, indices);
}
