#ifndef BEZIERCURVE_HPP
#define BEZIERCURVE_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <Camera.hpp>
#include <string>
#include <vector>
#include <shader.hpp>

class BezierCurve {
private:
    std::vector<glm::vec3> controlPoints; // Points de contrôle
    std::vector<glm::vec3> curvePoints;   // Points calculés de la courbe
    unsigned int resolution;              // Résolution de la courbe
    float size;
    GLuint VAO, VBO;                      // Buffers pour OpenGL
    GLuint shaderProgram;                 // Programme shader utilisé

    void generateCurvePoints();           // Calcul des points de la courbe

public:
    BezierCurve(unsigned int res = 50, float sz = 10);
    ~BezierCurve();

    void addControlPoint(const glm::vec3& point);
    void clearControlPoints();
    void setResolution(unsigned int new_res);
    void setSize(float new_size);
    void update(float newSize, unsigned int newResolution);
    void draw(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
};

#endif // BEZIERCURVE_HPP
