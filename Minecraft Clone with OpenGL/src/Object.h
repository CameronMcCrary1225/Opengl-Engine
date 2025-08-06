#ifndef OBJECT_H
#define OBJECT_H

#include <GL/glew.h>
#include <vector>
#include <glm/glm.hpp>
#include "Vertex.h"

// Primitive types supported by Object
enum class PrimitiveType {
    Cube,
    Plane,
    Sphere
};

class Object {
public:
    // Constructor: choose primitive type and detail level (for sphere subdivisions)
    Object(PrimitiveType type = PrimitiveType::Cube, int detail = 20);
    Object(const std::vector<Vertex>& verts,const std::vector<unsigned int>& inds);
    ~Object();

    // Draw the object (assumes shader is bound)
    void Draw() const;

    void getVAO();
    //void getIndicies();

    void getVertices(std::vector<Vertex>& outVerts) const { outVerts = vertices; }
    void getIndices(std::vector<unsigned int>& outInds) const { outInds = indices; }
    void getMeshData(
        std::vector<Vertex>& outVerts,
        std::vector<unsigned int>& outInds
    ) const {
        getVertices(outVerts);
        getIndices(outInds);
    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    unsigned int getIndexCount() const { return indexCount; }

private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // Initialization routines for each primitive
    void initCube();
    void initPlane();
    void initSphere(int sectors, int stacks);
    void initCustomMesh(const std::vector<Vertex>& verts,const std::vector<unsigned int>& inds);

};

#endif // OBJECT_H