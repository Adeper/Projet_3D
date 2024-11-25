#ifndef PLANE_HPP
#define PLANE_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <Camera.hpp>
#include <string>
#include <vector>

class Plane {
public:
    Plane(float size = 1.0f, unsigned int resolution = 10, Camera* cam = nullptr);
    ~Plane();

    void draw();
    void drawNormals();
    void update();

    int getResolution();

    void setHeightMap(GLuint heightMapID);

private:
    void createPlaneVAO();
    GLuint loadTexture(const std::string& texturePath);
    void showImGuiInterface();
    void updateSize(float newSize);
    void updateResolution(unsigned int newResolution);
    void recreatePlane();

    void initLodFBO();
    void renderLod();
    void showImGuiLOD();

    Camera* camera_plan; 

    float size;
    int resolution;
    float heightScale;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> uvs;
    std::vector<float> normals;
    glm::vec3 color;

    bool isWireframe;
    bool showNormals;

    GLuint VAO, VBO, EBO, UVBO, NBO, lodFBO, lodTexture;

    GLuint m_shaderProgram, m_normalShaderProgram, m_textureID, m_heightMapID, m_ColorID, m_lodShaderProgram;

    unsigned int m_indexCount;
};

#endif