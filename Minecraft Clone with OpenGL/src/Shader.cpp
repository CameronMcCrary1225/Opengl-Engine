#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
// Constructor that loads and compiles the shaders
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // Load shader source code from files
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        std::cerr << "ERROR: Shader source code loading failed!" << std::endl;
        return;
    }

    // Compile shaders and link them into a program
    compileShaders(vertexSource.c_str(), fragmentSource.c_str());
}

// Load shader source code from file
std::string Shader::loadShaderSource(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open shader file " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Compile and link shaders
void Shader::compileShaders(const char* vertexSource, const char* fragmentSource) {
    // Compile vertex shader
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glCompileShader(vertex);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cerr << "ERROR: Vertex Shader Compilation Failed\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);
    glCompileShader(fragment);

    // Check for compilation errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cerr << "ERROR: Fragment Shader Compilation Failed\n" << infoLog << std::endl;
    }

    // Link shaders into a program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    // Check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader Program Linking Failed\n" << infoLog << std::endl;
    }

    // Delete the individual shaders once they're linked into the program
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// Activate the shader program
void Shader::use() {
    glUseProgram(ID);
}

// Set uniform bool
void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

// Set uniform int
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

// Set uniform float
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

// Set uniform mat4 (for transformations)
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "Warning: uniform '" << name << "' not found\n";
    }
    else {
        glUniform3f(loc, x, y, z);
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& v) const {
    setVec3(name, v.x, v.y, v.z);
}
