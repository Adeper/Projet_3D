#ifndef BEZIERCURVE_HPP
#define BEZIERCURVE_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <Camera.hpp>
#include <string>
#include <vector>

class BezierCurve {
public:
    BezierCurve(int resolution, Camera* cam);
    ~BezierCurve();
    void computeCurve();
    const std::vector<glm::vec3>& getCurvePoints() const;
    void draw();
    void setResolution(unsigned int new_res);
    void setHeightData(const std::vector<float>& heightData);
    void initControlPoints(const glm::vec3& startPoint, const glm::vec3& endPoint, int nbControlPoints);
    void update();

private:
    std::vector<glm::vec3> controlPoints; // Points de contrôle
    std::vector<glm::vec3> curvePoints;   // Points sur la courbe
    unsigned int resolution;
    Camera* camera;
    std::vector<float> m_heightData;

    GLuint VAO, VBO;                      // OpenGL buffers
    GLuint shaderProgram;               

    // Algorithme de Casteljau
    glm::vec3 deCasteljau(float t) const;
};

#endif
