#pragma once
#ifndef NODEFUNCTIONS_H
#define NODEFUNCTIONS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Vertex.h"
// Simple matrix-based node functions

// Translate a model matrix by delta
inline void translateModel(glm::mat4& model, const glm::vec3& delta) {
    model = glm::translate(model, delta);
}

// Scale a model matrix uniformly
inline void scaleModel(glm::mat4& model, float factor) {
    model = glm::scale(model, glm::vec3(factor));
}

// Rotate a model matrix around an axis by angle (radians)
inline void rotateModel(glm::mat4& model, float angleRadians, const glm::vec3& axis) {
    model = glm::rotate(model, angleRadians, axis);
}

// Reset a model matrix to identity
inline void resetModel(glm::mat4& model) {
    model = glm::mat4(1.0f);
}

inline std::vector<std::unique_ptr<Object>>
ArrayNodeObjects(const Object& prototype, int rows, int cols) {
    std::vector<std::unique_ptr<Object>> list;
    list.reserve(rows * cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Use the copy constructor to clone geometry & buffers
            auto obj = std::make_unique<Object>(prototype);

            // Build each instance’s transform
            glm::mat4 model = glm::mat4(1.0f);
            translateModel(model, glm::vec3(float(c), 0.0f, float(r)));

            // Store it on the object
            obj->modelMatrix = model;

            list.push_back(std::move(obj));
        }
    }

    return list;
}
//White Noise - Random Noise
void WhiteNoise(std::vector<std::unique_ptr<Object>> &objects, int rows, int cols,float seed, float heightScale) {
    std::srand(seed * 1295 + seed * 73737);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;

            float noiseValue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            float y_pos = noiseValue * heightScale;

            glm::mat4 model = objects[index]->modelMatrix;
            glm::vec3 translation = glm::vec3(model[3]);
            translation.y = y_pos;

            model[3] = glm::vec4(translation, 1.0f);
            objects[index]->modelMatrix = model;
        }
    }
}

inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

inline float fade(float t) {
    return t * t * (3.0f - 2.0f * t);
}

//Value Noise
void ValueNoise(std::vector<std::unique_ptr<Object>>& objects, int rows, int cols, float seed, float heightScale) {
    std::srand(seed * 1295 + seed * 73737);
    
    int gw = rows + 1, gh = cols + 1;
    std::vector<float> grid(gw * gh);
    for (int z = 0; z < gw; ++z) {
        for (int x = 0; x < gh; ++x) {
            grid[x + z * gw] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
    }
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            int index = i*cols + j;
            float x = float(i);
            float z = float(j);

            int x0 = int(std::floor(x));
            int z0 = int(std::floor(z));
            int x1 = x0 + 1;
            int z1 = z0 + 1;
            
            float tx = x - x0;
            float tz = z - z0;
            float u = fade(tx);
            float v = fade(tz);

            x0 = glm::clamp(x0, 0, gw - 1);
            z0 = glm::clamp(z0, 0, gh - 1);
            x1 = glm::clamp(x1, 0, gw - 1);
            z1 = glm::clamp(z1, 0, gh - 1);

            float v00 = grid[x0 + z0 * gw];
            float v10 = grid[x1 + z0 * gw];
            float v01 = grid[x0 + z1 * gw];
            float v11 = grid[x1 + z1 * gw];

            float i0 = lerp(v00, v10, u);
            float i1 = lerp(v01, v11, u);
            float noiseValue = lerp(i0, i1, v);

            float y_pos = noiseValue * heightScale;

            glm::mat4 model = objects[index]->modelMatrix;
            glm::vec3 translation = glm::vec3(model[3]);
            translation.y = y_pos;
            model[3] = glm::vec4(translation, 1.0f);
            objects[index]->modelMatrix = model;
        }
    }
}
void MinecraftNode(std::vector<std::unique_ptr<Object>>& objects, float blockSize) {
    for (auto& obj : objects) {
        glm::mat4& model = obj->modelMatrix;
        glm::vec3 translation = glm::vec3(model[3]);

        translation.y = std::round(translation.y / blockSize) * blockSize;

        model[3] = glm::vec4(translation, 1.0f);
    }
}

std::vector<std::unique_ptr<Object>> FillColumns(const std::vector<std::unique_ptr<Object>>& tops,int rows,int cols,float fillDepth,float blockSize = 1.0f) {
    std::vector<std::unique_ptr<Object>> filled;
    filled.reserve(rows * cols * (int)(fillDepth / blockSize + 1));

    for (const auto& top : tops) {
        // read top position
        glm::vec3 t = glm::vec3(top->modelMatrix[3]);
        // how many layers to generate
        int layers = (int)std::floor(t.y / blockSize);
        int maxLayers = (int)std::floor(fillDepth / blockSize);
        layers = glm::clamp(layers, 0, maxLayers);

        for (int k = 0; k <= layers; ++k) {
            float yk = k * blockSize;
            // clone geometry
            auto block = std::make_unique<Object>(*top);
            // build modelMatrix at (t.x, yk, t.z)
            glm::mat4 m(1.0f);
            m = glm::translate(m, glm::vec3(t.x, yk, t.z));
            block->modelMatrix = m;
            filled.push_back(std::move(block));
        }
    }
    return filled;
}

void PerlinNoise(std::vector<std::unique_ptr<Object>>& objects, int rows, int cols, float seed, float heightScale) {
    std::srand(seed * 1295 + seed * 73737);
    
    int width = rows + 1;
    int height = cols + 1;

    std::vector<glm::vec2> grid(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float angle = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f * 3.14159265f;
            grid[x + y * width] = glm::vec2(std::cos(angle), std::sin(angle));
        }
    }
    float frequency = .1f;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            float fx = float(i) * frequency;
            float fz = float(j) * frequency;

            int x0 = int(std::floor(fx));
            int y0 = int(std::floor(fz));
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float tx = fx - float(x0);
            float ty = fz - float(y0);
            float u = fade(tx);
            float v = fade(ty);

            x0 = glm::clamp(x0, 0, width - 1);
            y0 = glm::clamp(y0, 0, height - 1);
            x1 = glm::clamp(x1, 0, width - 1);
            y1 = glm::clamp(y1, 0, height - 1);

            glm::vec2 g00 = grid[x0 + y0 * width];
            glm::vec2 g10 = grid[x1 + y0 * width];
            glm::vec2 g01 = grid[x0 + y1 * width];
            glm::vec2 g11 = grid[x1 + y1 * width];

            glm::vec2 d00 = glm::vec2(tx, ty);
            glm::vec2 d10 = glm::vec2(tx - 1, ty);
            glm::vec2 d01 = glm::vec2(tx, ty - 1);
            glm::vec2 d11 = glm::vec2(tx - 1, ty - 1);

            float dot00 = glm::dot(g00, d00);
            float dot10 = glm::dot(g10, d10);
            float dot01 = glm::dot(g01, d01);
            float dot11 = glm::dot(g11, d11);

            float ix0 = lerp(dot00, dot10, u);
            float ix1 = lerp(dot01, dot11, u);
            float value = lerp(ix0, ix1, v);

            // Option A: see raw value in [-1,1]
            // float y_pos = value * heightScale;

            // Option B: normalize into [0,1]
            float normalized = (value + 1.0f) * 0.5f;
            float y_pos = normalized * heightScale;

            // 5) Apply to cube’s Y
            glm::mat4 model = objects[index]->modelMatrix;
            glm::vec3 pos = glm::vec3(model[3]);
            pos.y = y_pos;
            model[3] = glm::vec4(pos, 1.0f);
            objects[index]->modelMatrix = model;
        }
    }
}
void GenerateTwoTrianglesMesh(
    std::vector<std::unique_ptr<Object>>& objects,
    int rows,
    int cols
) {
    // Sanity check
    if ((int)objects.size() != rows * cols) {
        std::cerr << "GenerateTwoTrianglesMesh: expected "
            << rows * cols << " objects, got "
            << objects.size() << "\n";
        return;
    }

    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    verts.reserve(rows * cols);
    indices.reserve((rows - 1) * (cols - 1) * 6);

    // 1) Extract positions
    for (int j = 0; j < cols; ++j) {
        for (int i = 0; i < rows; ++i) {
            int idx = j * rows + i;
            glm::vec3 pos = glm::vec3(objects[idx]->modelMatrix[3]);
            verts.emplace_back(pos);
        }
    }

    // 2) Build two triangles per cell *only* up to rows-1, cols-1
    for (int j = 0; j < cols - 1; ++j) {
        for (int i = 0; i < rows - 1; ++i) {
            // flat index of (i,j)
            int v0 = j * rows + i;          // (i,   j)
            int v1 = v0 + 1;                // (i+1, j)
            int v2 = v0 + rows;             // (i,   j+1)
            int v3 = v2 + 1;                // (i+1, j+1)

            // debug check
            assert(v0 < (int)verts.size());
            assert(v1 < (int)verts.size());
            assert(v2 < (int)verts.size());
            assert(v3 < (int)verts.size());

            // Triangle A
            indices.push_back(v0);
            indices.push_back(v2);
            indices.push_back(v1);

            // Triangle B
            indices.push_back(v1);
            indices.push_back(v2);
            indices.push_back(v3);
        }
    }

    // 3) Compute normals exactly as before
    for (auto& v : verts) v.normal = glm::vec3(0.0f);
    for (size_t k = 0; k < indices.size(); k += 3) {
        unsigned i0 = indices[k + 0], i1 = indices[k + 1], i2 = indices[k + 2];
        const auto& p0 = verts[i0].position;
        const auto& p1 = verts[i1].position;
        const auto& p2 = verts[i2].position;
        glm::vec3 fn = glm::normalize(glm::cross(p1 - p0, p2 - p0));
        verts[i0].normal += fn;
        verts[i1].normal += fn;
        verts[i2].normal += fn;
    }
    for (auto& v : verts) v.normal = glm::normalize(v.normal);

    // 4) Build the mesh
    auto terrain = std::make_unique<Object>(verts, indices);
    objects.clear();
    objects.push_back(std::move(terrain));
}


#endif // NODEFUNCTIONS_H