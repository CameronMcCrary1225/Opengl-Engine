#version 330 core

// Per-vertex attributes for the base blade mesh
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

// Output to fragment shader
out vec3 vColor;

// Uniforms from CPU
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D heightMap;    // R32F texture containing heightfield
uniform float chunkSize;        // world-space width/depth of chunk
uniform int   gridWidth;       // nx
uniform int   gridHeight;      // nz
uniform float maxHeight;       // amplitude
uniform vec2  chunkOrigin;     // world-space XZ origin for this chunk

// blade variation
uniform float rotationVariance; // in radians
uniform float scaleVariance;    // as fraction (0.3 => ±30%)

// optional tint multiplier (set if you want)
uniform vec3 grassTint;

//
// small deterministic hash-based pseudo-random for each instance
//
float prand(int x, int y) {
    // convert to float and produce deterministic pseudo random in [0,1)
    float xf = float(x);
    float yf = float(y);
    float n = sin(dot(vec2(xf, yf), vec2(12.9898,78.233))) * 43758.5453123;
    return fract(n);
}

void main() {
    // compute grid coords from instance id
    int id = gl_InstanceID;
    int ix = id % gridWidth;
    int iz = id / gridWidth;

    // UV across height texture (avoid division by zero)
    vec2 denom = vec2(max(1, gridWidth - 1), max(1, gridHeight - 1));
    vec2 uv = vec2(float(ix) / denom.x, float(iz) / denom.y);

    // sample height (heightMap stores normalized [0..1], multiply by maxHeight)
    float h = texture(heightMap, uv).r * maxHeight;

    // compute base world XZ for this instance
    vec2 worldXZ = chunkOrigin + uv * chunkSize;

    // deterministic pseudo-random value for this instance
    float r = prand(ix, iz);           // [0,1)
    float angle = (r * 2.0 - 1.0) * rotationVariance; // [-rotVar, rotVar]
    float scl = 1.0 + (r * 2.0 - 1.0) * scaleVariance; // 1 ± scaleVariance

    // rotate aPos.xz by angle, scale by scl
    float c = cos(angle);
    float s = sin(angle);
    vec2 posXZ = aPos.xz * scl;
    vec2 rotatedXZ = vec2(c * posXZ.x - s * posXZ.y,
                          s * posXZ.x + c * posXZ.y);

    // final world-space vertex position
    vec3 worldPos = vec3(worldXZ.x + rotatedXZ.x,
                         h + aPos.y * scl,
                         worldXZ.y + rotatedXZ.y);

    gl_Position = projection * view * vec4(worldPos, 1.0);

    // final vertex color (mesh color modulated by tint)
    vColor = aColor * grassTint;
}
