#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <numeric>
#include <random>
extern Shader grassShader;

namespace Tera {

	struct GridPoints {
		int nx = 0;
		int nz = 0;
		std::vector<glm::vec2> pts;
};

	GridPoints make_grid(int cx, int cz, int nx, int nz, float chunkSize) {
		GridPoints terrain;
		terrain.nx = nx;
		terrain.nz = nz;
		terrain.pts.reserve(nx * nz);

		for (int z = 0; z < nz; ++z) {
			for (int x = 0; x < nx; ++x) {
				float u = (nx > 1) ? (static_cast<float>(x) / (nx - 1)) : 0.0f;
				float v = (nz > 1) ? (static_cast<float>(z) / (nz - 1)) : 0.0f;
				float wx = cx * chunkSize + u * chunkSize;
				float wz = cz * chunkSize + v * chunkSize;
				terrain.pts.emplace_back(wx, wz);
			}
		}
		return terrain;

	}

	struct HeightField {
		int nx = 0;
		int nz = 0;
		std::vector<float> heights;
};

    inline float perlin_sample(float x, float z,
        const std::vector<int>& perm) {
        // Locate unit grid cell
        int X = int(std::floor(x)) & 255;
        int Z = int(std::floor(z)) & 255;
        float xf = x - std::floor(x);
        float zf = z - std::floor(z);
        // Fade curves
        auto fade = [](float t) { return t * t * t * (t * (t * 6 - 15) + 10); };
        float u = fade(xf);
        float v = fade(zf);
        // Gradient helper
        auto grad = [](int hash, float dx, float dz) {
            switch (hash & 0x3) {
            case 0: return  dx + dz;
            case 1: return -dx + dz;
            case 2: return  dx - dz;
            case 3: return -dx - dz;
            }
            return 0.0f;
            };
        // Hash corner indices
        int aa = perm[perm[X] + Z];
        int ab = perm[perm[X] + Z + 1];
        int ba = perm[perm[X + 1] + Z];
        int bb = perm[perm[X + 1] + Z + 1];
        // Linear interp
        auto lerp = [](float a, float b, float t) { return a + t * (b - a); };
        float x1 = lerp(grad(aa, xf, zf), grad(ba, xf - 1, zf), u);
        float x2 = lerp(grad(ab, xf, zf - 1), grad(bb, xf - 1, zf - 1), u);
        return lerp(x1, x2, v);
    }
    inline std::vector<int> build_permutation(int seed) {
        std::vector<int> p(256);
        for (int i = 0; i < 256; ++i) p[i] = i;
        std::srand(seed * 1295 + seed * 73737);
        for (int i = 255; i > 0; --i) {
            int j = std::rand() % (i + 1);
            std::swap(p[i], p[j]);
        }
        p.insert(p.end(), p.begin(), p.end());
        return p;
    }
    inline std::vector<float> noise2d(const GridPoints& grid,
        float frequency,
        int seed) {
        auto perm = build_permutation(seed);
        std::vector<float> out;
        out.reserve(grid.pts.size());
        for (auto& p : grid.pts) {
            float x = p.x * frequency;
            float z = p.y * frequency;
            out.push_back(perlin_sample(x, z, perm));
        }
        return out;
    }
    inline std::vector<float> flat_noise(const GridPoints& grid,
        float value = 0.0f) {
        std::vector<float> out(grid.pts.size(), value);
        return out;
    }
    inline HeightField make_heightfield(int nx, int nz,
        const std::vector<float>& noise,
        float amplitude) {
        HeightField h; h.nx = nx; h.nz = nz;
        h.heights.resize(noise.size());
        for (size_t i = 0; i < noise.size(); ++i) {
            float n = (noise[i] + 1.0f) * 0.5f; // [0,1]
            h.heights[i] = n * amplitude;
        }
        return h;
    }
    inline void ColorMeshByHeight(std::vector<Vertex>& verts, const HeightField& hf, float waterLevel, float sandLevel, float grassLevel, float rockLevel) {
        size_t N = verts.size();
        for (size_t i = 0; i < N; ++i) {
            float h = hf.heights[i];
            glm::vec3 c;
            if (h < waterLevel) {
                c = glm::vec3(0.0, 0.0, 0.5);
            }
            else if (h < sandLevel) {
                c = glm::vec3(0.76, 0.7, 0.5);
            }
            else if (h < grassLevel) {
                c = glm::vec3(0.2, 0.6, 0.2);
            }
            else if (h < rockLevel) {
                c = glm::vec3(0.5, 0.5, 0.5);
            }
            else {
                c = glm::vec3(1, 1, 1);

            }
            verts[i].color = c;
        }
    }
    inline std::unique_ptr<Object> GenerateTwoTrianglesMesh(const GridPoints& grid, const HeightField& hf
    ) {
        int rows = grid.nx;
        int cols = grid.nz;
        int N = rows * cols;

        // 1) Extract positions into Vertex list
        std::vector<Vertex> verts;
        verts.reserve(N);
        for (int j = 0; j < cols; ++j) {
            for (int i = 0; i < rows; ++i) {
                int idx = j * rows + i;
                glm::vec3 pos(
                    grid.pts[idx].x,
                    hf.heights[idx],
                    grid.pts[idx].y
                );
                verts.emplace_back(pos);
            }
        }

        // 2) Build two triangles per cell
        std::vector<unsigned int> indices;
        indices.reserve((rows - 1) * (cols - 1) * 6);
        for (int j = 0; j < cols - 1; ++j) {
            for (int i = 0; i < rows - 1; ++i) {
                int v0 = j * rows + i;
                int v1 = v0 + 1;
                int v2 = v0 + rows;
                int v3 = v2 + 1;
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

        // 3) Compute normals
        for (auto& v : verts) v.normal = glm::vec3(0.0f);
        for (size_t k = 0; k < indices.size(); k += 3) {
            unsigned i0 = indices[k + 0];
            unsigned i1 = indices[k + 1];
            unsigned i2 = indices[k + 2];
            const glm::vec3& p0 = verts[i0].position;
            const glm::vec3& p1 = verts[i1].position;
            const glm::vec3& p2 = verts[i2].position;
            glm::vec3 fn = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            verts[i0].normal += fn;
            verts[i1].normal += fn;
            verts[i2].normal += fn;
        }
        for (auto& v : verts) v.normal = glm::normalize(v.normal);
        if (false) {//enable color boolean option for later
            ColorMeshByHeight(verts, hf, 0, 2, 5, 8);
        }
        // 4) Create Object
        return std::make_unique<Object>(verts, indices);
    }
    inline std::vector<float> FractalNoise(const GridPoints& grid, float baseFreq, float baseAmp, int octaves, float persistence, int seed) {
        std::vector<float> result(grid.pts.size(), 0.0f);
        for (int i = 0; i < octaves; ++i) {
            float freq = baseFreq * static_cast<float>(std::pow(2.0, i));
            float amp = baseAmp * static_cast<float>(std::pow(persistence, i));
            auto layer = noise2d(grid, freq, seed + i);
            for (size_t j = 0; j < result.size(); ++j)
                result[j] += layer[j] * amp;
        }
        return result;
    }
    inline float RandRange(float min, float max) {
        float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        return min + r * (max - min);
    }
    inline std::unique_ptr<Object> ScatterObjects(const Object& baseObject, const Object& terrain, float scatterDensity, float scaleVariance, float rotationVariance) {
        static bool seeded = false;
        if (!seeded) {
            std::srand((unsigned)std::time(nullptr));
            seeded = true;
        }

        // Extract terrain vertex positions
        std::vector<Vertex> terrainVerts;
        std::vector<unsigned int> dummyInds;
        terrain.getMeshData(terrainVerts, dummyInds);

        // Extract base mesh data
        std::vector<Vertex> baseVerts;
        std::vector<unsigned int> baseInds;
        baseObject.getMeshData(baseVerts, baseInds);
        std::cout << "Scattered mesh: "
            << baseVerts.size() << " verts, "
            << (baseInds.size() / 3) << " triangles\n";
        // Prepare combined buffers
        std::vector<Vertex> verts;
        std::vector<unsigned int> inds;
        verts.reserve(terrainVerts.size() * scatterDensity);
        inds.reserve(terrainVerts.size() * baseInds.size() * scatterDensity);

        // Loop over terrain vertices
        for (size_t vi = 0; vi < terrainVerts.size(); ++vi) {
            if (static_cast<float>(std::rand()) / RAND_MAX > scatterDensity)
                continue;
            const glm::vec3& pos = terrainVerts[vi].position;

            // Random transform
            float angle = ((float)std::rand() / RAND_MAX * 2.0f - 1.0f) * rotationVariance;
            float scl = 1.0f + ((float)std::rand() / RAND_MAX * 2.0f - 1.0f) * scaleVariance;

            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
            model = glm::scale(model, glm::vec3(scl));

            // Append base mesh under this transform
            unsigned int offset = static_cast<unsigned int>(verts.size());
            for (auto const& bv : baseVerts) {
                Vertex vt = bv;
                vt.position = glm::vec3(model * glm::vec4(bv.position, 1.0f));
                vt.normal = glm::mat3(glm::transpose(glm::inverse(model))) * bv.normal;
                verts.push_back(vt);
            }
            for (auto bi : baseInds) {
                inds.push_back(offset + bi);
            }
        }

        // Return merged scattered mesh
        return std::make_unique<Object>(verts, inds);
    }
    inline GLuint CreateHeightMapTexture(const HeightField& hf, int nx, int nz) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_R32F,
            nx, nz,
            0,
            GL_RED,
            GL_FLOAT,
            hf.heights.data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return tex;
    }

    inline void Chunks(std::vector<std::unique_ptr<Object>>& objects, int centerCx, int centerCz, int radius, int nx, int nz, float chunkSize, float frequency, float amplitude, int seed, int octaves, float persistence, Shader grassShader, std::vector<GLuint>& outHeightTextures, std::vector<glm::vec2>& outChunkOrigins) {
        for (int dz = -radius; dz <= radius; ++dz) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int cx = centerCx + dx;
                int cz = centerCz + dz;

                GridPoints grid = make_grid(cx, cz, nx, nz, chunkSize);
                //std::vector<float> raw = noise2d(grid, frequency, seed);
                std::vector<float> raw = FractalNoise(grid, frequency, amplitude, octaves, persistence, seed);
                

                HeightField hf = make_heightfield(nx, nz, raw, amplitude);
                GLuint heightTex = Tera::CreateHeightMapTexture(hf, nx, nz);
                //Tera::BindHeightMapUniforms(grassShader,heightTex,0,chunkSize,nx, nz,amplitude,glm::vec2(cx * chunkSize, cz * chunkSize));
                outHeightTextures.push_back(heightTex);
                outChunkOrigins.emplace_back(cx * chunkSize, cz * chunkSize);
                
                auto mesh = GenerateTwoTrianglesMesh(grid, hf);
                
                objects.push_back(std::move(mesh));
            }
        }
    }
}