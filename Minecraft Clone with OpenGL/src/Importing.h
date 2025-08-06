#pragma once

#include <string>
#include <vector>
#include <utility>
#include "Vertex.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include "Object.h"
class OBJModel {
public:
    OBJModel() = default;

    /// Load an Obj from disk. Returns false on failure.
    bool load(const std::string& path);

    /// Build an Object from the loaded data (positions + normals).
    /// Returns nullptr if load() hasn’t been called or failed.
    std::unique_ptr<Object> buildObject() const;

private:
    // raw data from OBJ
    std::vector<glm::vec3> positions_;
    std::vector<glm::vec3> normals_;
    std::vector<unsigned int> posIndices_;
    std::vector<unsigned int> normIndices_;
};