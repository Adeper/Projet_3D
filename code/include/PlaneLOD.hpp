#ifndef PLANELOD_HPP
#define PLANELOD_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <Camera.hpp>
#include <string>
#include <vector>
#include <array>

class PlaneLOD {
public:
    PlaneLOD(float size = 1.0f, unsigned int resolution = 10, Camera* cam = nullptr);
    ~PlaneLOD();

    void draw();
    void drawNormals();
    void update();

    int getResolution() const;
    float getSize() const;
    float getHeightScale() const;

    void setResolution(float resolution);
    void setSize(float size);
    void setHeightScale(float heightScale);

    int getLodDistance() const;
    int getMorphFactor() const;
    int getMorphDistance() const;

    void setHeightMap(GLuint heightMapID);
    GLuint getHeightMap() const;
    void showImGuiInterface();

    void recreatePlane();

    void debugImgui();

private:
    void createPlaneVAO();
    GLuint loadTexture(const std::string &path);
    void updateSize(float newSize);
    void updateResolution(unsigned int newResolution);
    void initLight();
    void updateLightRotation();
    void reloadShaders();
    

    Camera* camera_plan; 

    float size;
    int resolution;
    float heightScale;
    float maxLodDistance;
    float morphFactor;
    float morphDistance;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> uvs;
    std::vector<float> normals;
    glm::vec3 color;

    bool displayWire;
    bool displayPoint;
    bool showNormals;

    GLuint VAO, VBO, EBO, UVBO, NBO, lodFBO, lodTexture, IDBO;

    GLuint m_shaderProgram, m_normalShaderProgram, m_textureID, m_heightMapID;

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

    // LOD
    float blendFactors[3] = {1.0f, 0.0f, 0.0f}; // Facteurs de mélange pour chaque niveau de LOD

};

#endif