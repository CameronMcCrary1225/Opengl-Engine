#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include "ALLHeaders.h"
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // GLFW init
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Camera", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    // Camera
    Camera camera;
    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, Camera::mouse_callback);
    glfwSetScrollCallback(window, Camera::scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    // Shader
    Shader ourShader("res/shader/shader.vert", "res/shader/shader.frag");
    Shader grassShader("res/shader/grass.vert", "res/shader/grass.frag");

    // A simple vertical quad (2 triangles = 6 indices)
    std::vector<Vertex> bladeVerts = {
        // pos            // normal       // color
        {{-0.05f, 0.0f,0},{0,1,0},{0.1f,0.8f,0.1f}},
        {{ 0.05f, 0.0f,0},{0,1,0},{0.1f,0.8f,0.1f}},
        {{-0.05f, 1.0f,0},{0,1,0},{0.1f,0.8f,0.1f}},
        {{ 0.05f, 1.0f,0},{0,1,0},{0.1f,0.8f,0.1f}},
    };
    std::vector<unsigned int> bladeInds = {
        0,2,1,
        1,2,3
    };
    auto grassBlade = std::make_unique<Object>(bladeVerts, bladeInds);

    // Objects (unique_ptr avoids shallow copies)
    std::vector<std::unique_ptr<Object>> objlist;
   
    int chunkSize = 10;
    int centerCx = static_cast<int>(camera.Position.x / chunkSize);
    int centerCy = static_cast<int>(camera.Position.y / chunkSize);

    //Tera::Chunks(objlist, centerCx, centerCx, 2, 50, 50, 100.0f, .1f, 1.0f, 1, 8, 1);
    std::vector<GLuint> chunkHeightTextures;
    std::vector<glm::vec2> chunkOrigins;
    Tera::Chunks(objlist, centerCx, centerCy, 2, 100, 100, 100.0f, .05f, 2.0f, 1, 4, .3, grassShader, chunkHeightTextures, chunkOrigins);

    auto lastTime = std::chrono::high_resolution_clock::now();
    int frames = 0;
    size_t totalVerts = 0;
    while (!glfwWindowShouldClose(window)) {
        // Time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        camera.ProcessInput(window, deltaTime);
        // Track last known chunk coordinates
        static int lastCx = INT32_MIN;
        static int lastCz = INT32_MIN;

        // Determine current chunk coordinates
        int chunkSize = 10;
        int currentCx = static_cast<int>(camera.Position.x / chunkSize);
        int currentCz = static_cast<int>(camera.Position.z / chunkSize);

        // Only regenerate chunks if player moved to a new chunk
        if (currentCx != lastCx || currentCz != lastCz) {
            lastCx = currentCx;
            lastCz = currentCz;
            std::cout << "chunks regenerated" << currentCx << currentCz << std::endl;
            objlist.clear(); // Clear old terrain

            // Generate new terrain centered around the player
            //Tera::Chunks(objlist, currentCx, currentCz, 2, 100, 100, 100.0f, .05f, 4.0f, 1, 8, .3);
            Tera::Chunks(objlist, currentCx, currentCz, 2, 100, 100, 100.0f, .05f, 2.0f, 1, 4, .3, grassShader, chunkHeightTextures, chunkOrigins);

        }
        // Clear
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activate shader and set view/projection
        ourShader.use();
        //ourShader.setVec3("lightPos", glm::vec3(0.0f, 10.0f, 10.0f));
        //ourShader.setVec3("lightColor", glm::vec3(5.0f));
        //ourShader.setVec3("objectColor", glm::vec3(0.8f, 0.3f, 0.3f));

        glm::vec3 sunDirection = glm::normalize(glm::vec3(-0.1f, -1.0f, -0.1f)); // pointing down and slightly angled
        ourShader.setVec3("lightPos", sunDirection);
        ourShader.setVec3("lightColor", glm::vec3(1.0f, .95f, .8f));



        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            800.0f / 600.0f,
            0.1f, 500.0f
        );
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        for (auto& obj : objlist) {
            ourShader.setMat4("model", obj->modelMatrix);
            obj->Draw();
        }
        grassShader.use();

        // reset all of the grass?shader uniforms for this frame
        grassShader.setMat4("view", view);
        grassShader.setMat4("projection", projection);
        for (size_t i = 0; i < objlist.size(); ++i) {
            GLuint heightTex = chunkHeightTextures[i];
            glm::vec2 origin = chunkOrigins[i];

            // debug-check: is the texture valid?
            if (!glIsTexture(heightTex)) {
                std::cerr << "Warning: invalid height texture at chunk " << i << "\n";
                continue;
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, heightTex);
            grassShader.setInt("heightMap", 0);

            // set uniforms for this chunk
            grassShader.setFloat("chunkSize", 100.0f); // or your variable
            grassShader.setInt("gridWidth", 100);      // nx
            grassShader.setInt("gridHeight", 100);     // nz
            grassShader.setFloat("maxHeight", 2.0f);   // amplitude
            grassShader.setVec2("chunkOrigin", origin);

            GLsizei bladeCount = 100 * 100; // or nx * nz
            grassBlade->DrawInstanced(bladeCount);

            // optional: check GL error
            GLenum err = glGetError();
            if (err != GL_NO_ERROR) {
                std::cerr << "GL error after binding/draw: " << std::hex << err << std::dec << "\n";
            }
        }
        //genereate framerate to consol
        frames++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;

        if (elapsed.count() >= 1.0f) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastTime = currentTime;
        }

        // Swap & poll
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
