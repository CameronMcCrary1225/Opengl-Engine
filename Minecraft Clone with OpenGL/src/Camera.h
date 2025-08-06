#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
    // Camera attributes
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch;
    float MovementSpeed, MouseSensitivity, Zoom;

    // Constructor with vector parameters
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f);

    // Returns the view matrix calculated using Euler Angles and the LookAt matrix
    glm::mat4 GetViewMatrix() const;

    // Processes keyboard input for camera movement
    void ProcessInput(GLFWwindow* window, float deltaTime);
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Processes mouse movement input
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Processes mouse scroll input
    void ProcessMouseScroll(float yoffset);

    // GLFW callbacks
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};

#endif // CAMERA_H