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

    int getResolution() const;
    float getSize() const;
    float getHeightScale() const;

    void setHeightMap(GLuint heightMapID);

private:
    void createPlaneVAO();
    GLuint loadTexture(const std::string &path);
    void showImGuiInterface();
    void updateSize(float newSize);
    void updateResolution(unsigned int newResolution);
    void recreatePlane();
    void initLight();
    void updateLightRotation();

    void initLodFBO();
    void renderLod();
    void showImGuiLOD();

    Camera* camera_plan; 

    float size;
    int resolution;
    float heightScale;
    float maxLodDistance;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> uvs;
    std::vector<float> normals;
    glm::vec3 color;

    bool displayWire;
    bool displayPoint;
    bool showNormals;

    GLuint VAO, VBO, EBO, UVBO, NBO, lodFBO, lodTexture, IDBO;

    GLuint m_shaderProgram, m_normalShaderProgram, m_textureID, m_heightMapID, m_lodShaderProgram;

    GLuint m_grassTextureID;
    GLuint m_rockTextureID;
    GLuint m_snowTextureID;

    float grassLimit;
    float rockLimit;

    /*==Variables de la lumière==*/
    glm::vec3 lightDirection;
    glm::vec3 lightColor;
    glm::vec3 ambientColor;
    float lightRotationAngle;

    unsigned int m_indexCount;
};

#endif