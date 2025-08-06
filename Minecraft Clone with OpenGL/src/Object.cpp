#include "Object.h"
#include <glm/gtc/constants.hpp>
#include <iostream>

Object::Object(PrimitiveType type, int detail) {
    switch (type) {
    case PrimitiveType::Cube:
        initCube();
        break;
    case PrimitiveType::Plane:
        initPlane();
        break;
    case PrimitiveType::Sphere:
        initSphere(detail, detail);
        break;
    }
}

Object::Object(const std::vector<Vertex>& verts, const std::vector<unsigned int>& indices)
{
    modelMatrix = glm::mat4(1.0f);
    initCustomMesh(verts, indices);
}

Object::~Object() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Object::Draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Object::getVAO()
{
    std::cout << "VAO" << VAO << std::endl;
}
/*
void Object::getIndicies()
{
    std::cout << "Indicies" << indexCount << std::endl;
}
*/
void Object::initCube() {
    static const float vertices[] = {
        // positions         // colors
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f
    };
    static const unsigned int indices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        4,5,1, 1,0,4,
        6,7,3, 3,2,6,
        4,7,3, 3,0,4,
        5,6,2, 2,1,5
    };

    indexCount = sizeof(indices) / sizeof(unsigned int);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Object::initPlane() {
    static const float vertices[] = {
        // positions        // colors
        -0.5f, 0.0f, -0.5f,  0.6f, 0.6f, 0.6f,
         0.5f, 0.0f, -0.5f,  0.6f, 0.6f, 0.6f,
         0.5f, 0.0f,  0.5f,  0.6f, 0.6f, 0.6f,
        -0.5f, 0.0f,  0.5f,  0.6f, 0.6f, 0.6f
    };
    static const unsigned int indices[] = {
        0,1,2,
        2,3,0
    };

    indexCount = sizeof(indices) / sizeof(unsigned int);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Object::initSphere(int sectors, int stacks) {
    std::vector<float> verts;
    std::vector<unsigned int> inds;
    float radius = 0.5f;
    float const PI = glm::pi<float>();
    float sectorStep = 2 * PI / sectors;
    float stackStep = PI / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        float xy = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);
            // position
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
            // color (use normal-based coloring)
            glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
            verts.push_back((n.x + 1.0f) * 0.5f);
            verts.push_back((n.y + 1.0f) * 0.5f);
            verts.push_back((n.z + 1.0f) * 0.5f);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        for (int j = 0; j < sectors; ++j) {
            if (i != 0) {
                inds.push_back(k1 + j);
                inds.push_back(k2 + j);
                inds.push_back(k1 + j + 1);
            }
            if (i != (stacks - 1)) {
                inds.push_back(k1 + j + 1);
                inds.push_back(k2 + j);
                inds.push_back(k2 + j + 1);
            }
        }
    }

    indexCount = static_cast<unsigned int>(inds.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(unsigned int), inds.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Object::initCustomMesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& indices)
{
    this->vertices = verts;
    this->indices = indices;
    indexCount = static_cast<unsigned int>(indices.size());

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex, position));

    // Enable and set up normal attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3, GL_FLOAT, GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex, normal));
    // Enable and set up color attribute (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);
}
