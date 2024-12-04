#include "BezierCurve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

BezierCurve::BezierCurve(unsigned int res, float sz) : resolution(res), size(sz)  {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    heightScale = 1.;

    shaderProgram = LoadShaders("bezier_vertex_shader.glsl", "bezier_fragment_shader.glsl");
}

BezierCurve::~BezierCurve() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void BezierCurve::initControlPoints(const glm::vec3& start, const glm::vec3& end, unsigned int nbControlPoints) {
    controlPoints.clear();
    addControlPoint(start);

    if (nbControlPoints > 0) {
        glm::vec3 direction = (end - start) / static_cast<float>(nbControlPoints + 1);
        for (unsigned int i = 1; i <= nbControlPoints; ++i) {
            glm::vec3 point = start + static_cast<float>(i) * direction;
            addControlPoint(point);
        }
    }

    addControlPoint(end);

    glGenBuffers(1, &controlPointsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, controlPointsBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, controlPoints.size() * sizeof(glm::vec3), controlPoints.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, controlPointsBuffer);
}

void BezierCurve::addControlPoint(const glm::vec3& point) {
    controlPoints.push_back(point);
}

void BezierCurve::clearControlPoints() {
    controlPoints.clear();
    curvePoints.clear();
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
}

void BezierCurve::setResolution(unsigned int new_res) {
    resolution = new_res;
    
}

void BezierCurve::setSize(float sz) {
    size = sz;
}

void BezierCurve::setHeightScale(float new_heightScale){
    heightScale = new_heightScale;
}

void BezierCurve::update(float new_size, unsigned int new_res, float new_heightScale){
    setResolution(new_size);
    setSize(new_res);
    setHeightScale(new_heightScale);
}

void BezierCurve::draw(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    if (curvePoints.empty()) return;

    glUseProgram(shaderProgram);

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projMatrix[0][0]);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    for (unsigned int i = 0; i <= static_cast<int>(size); ++i) {
        float t = static_cast<float>(i) / size;
        glUniform1f(glGetUniformLocation(shaderProgram, "t"), t);
        glDrawArrays(GL_POINTS, 0, 1);
    }

    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());

    glBindVertexArray(0);
}