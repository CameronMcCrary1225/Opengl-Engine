#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <glm/glm.hpp>


class Shader {
public:
    unsigned int ID; // Shader Program ID

    // Constructor: Loads and compiles the shaders
    Shader(const char* vertexPath, const char* fragmentPath);

    // Use the shader program
    void use();

    // Utility function to set uniforms
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, float x, float y, float z) const;  // ? changed
    void setVec3(const std::string& name, const glm::vec3& v) const;

private:
    // Helper function to load shader code from file
    std::string loadShaderSource(const char* path);

    // Compile and link shaders
    void compileShaders(const char* vertexSource, const char* fragmentSource);
};

#endif // SHADER_H