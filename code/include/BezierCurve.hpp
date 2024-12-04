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
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> curvePoints;
    unsigned int resolution;
    float size;
    float heightScale;
    GLuint VAO, VBO;
    GLuint controlPointsBuffer;
    GLuint shaderProgram;

    void addControlPoint(const glm::vec3& point);
    void updateControlPoints();

public:
    BezierCurve(unsigned int res = 50, float sz = 10);
    ~BezierCurve();

    void initControlPoints(const glm::vec3& start, const glm::vec3& end, unsigned int nbControlPoints);
    void clearControlPoints();
    void setResolution(unsigned int new_res);
    void setSize(float new_size);
    void setHeightScale(float new_heightScale);
    void update(float newSize, unsigned int newResolution, float new_heightScale);
    void draw(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
};

#endif // BEZIERCURVE_HPP
