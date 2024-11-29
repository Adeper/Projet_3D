#include "BezierCurve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

BezierCurve::BezierCurve(unsigned int res, float sz) : resolution(res), size(sz)  {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    shaderProgram = LoadShaders("bezier_vertex_shader.glsl", "bezier_fragment_shader.glsl");
}

BezierCurve::~BezierCurve() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void BezierCurve::addControlPoint(const glm::vec3& point) {
    controlPoints.push_back(point);
    generateCurvePoints();
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


void BezierCurve::update(float new_size, unsigned int new_res){
    setResolution(new_size);
    setSize(new_res);
    generateCurvePoints();
}

void BezierCurve::generateCurvePoints() {
    curvePoints.clear();

    // Algorithme de De Casteljau
    for (unsigned int i = 0; i <= static_cast<int>(size); ++i) {
        float t = static_cast<float>(i) / size;

        std::vector<glm::vec3> temp = controlPoints;
        while (temp.size() > 1) {
            std::vector<glm::vec3> next;
            for (size_t j = 0; j < temp.size() - 1; ++j) {
                glm::vec3 interpolated = (1.0f - t) * temp[j] + t * temp[j + 1];
                next.push_back(interpolated);
            }
            temp = next;
        }

        if (!temp.empty()) {
            curvePoints.push_back(temp[0] * static_cast<float>(resolution/10.));
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);
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

    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());

    glBindVertexArray(0);
}
