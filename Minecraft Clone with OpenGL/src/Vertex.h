#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    Vertex() = default;
    Vertex(const glm::vec3& pos)
        : position(pos), normal(0.0f, 1.0f, 0.0f),color(.2f,.6f,.2f) {}
    Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec3& col)
        : position(pos), normal(norm), color(col) {}
};