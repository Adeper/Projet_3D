#ifndef PLANE_HPP
#define PLANE_HPP

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

// Include ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Include shaders
#include <shader.hpp>

class Plane {

private:
    int width, height;       // Dimensions de la grille
    float spacing;           // Espacement entre les sommets
    float translateX, translateY, translateZ; // Translation
    float rotateX, rotateY, rotateZ; // Rotation
    GLuint shaderProgram; // ID du programme shader

    GLuint vao, vboVertices, vboIndices, vboNormals;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> normals;

    void generateVertices();
    void generateIndices();
    void generateNormals();
    void setupBuffers();
    void cleanUpBuffers();

public:
    Plane(int width = 10, int height = 10, float spacing = 1.0f);
    ~Plane();

    void init();
    void draw();
    void update(int newWidth, int newHeight);
    void imguiInterface();
    void loadShaders(const char* vertexPath, const char* fragmentPath);
};

#endif // PLANE_HPP
