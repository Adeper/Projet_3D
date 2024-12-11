#include "BezierCurve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <shader.hpp>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

BezierCurve::BezierCurve(int resolution, Camera* cam)
    : resolution(resolution), camera(cam), VAO(0), VBO(0) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    shaderProgram = LoadShaders("bezier_vertex_shader.glsl", "bezier_fragment_shader.glsl");
}

BezierCurve::~BezierCurve() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void BezierCurve::computeCurve() {
    curvePoints.clear();
    for (int i = 0; i <= resolution; ++i) {
        float t = static_cast<float>(i) / resolution;
        glm::vec3 point = deCasteljau(t);

        float height = m_heightData[point.x * resolution + point.z];
        point.y = height;

        curvePoints.push_back(point);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void BezierCurve::update(){
    computeCurve();
    draw();
}

void BezierCurve::draw() {
    glUseProgram(shaderProgram);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    const glm::mat4& viewMatrix = camera->getViewMatrix();
    const glm::mat4& projMatrix = camera->getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projMatrix[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());
    glBindVertexArray(0);

    glUseProgram(0);
}

glm::vec3 BezierCurve::deCasteljau(float t) const {
    std::vector<glm::vec3> points = controlPoints;
    while (points.size() > 1) {
        std::vector<glm::vec3> nextPoints;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            nextPoints.push_back((1 - t) * points[i] + t * points[i + 1]);
        }
        points = nextPoints;
    }
    return points[0];
}

void BezierCurve::initControlPoints(const glm::vec3& startPoint, const glm::vec3& endPoint, int nbControlPoints) {
    if (nbControlPoints < 2) {
        throw std::invalid_argument("Number of control points must be at least 2.");
    }

    controlPoints.clear();

    for (int i = 0; i < nbControlPoints; ++i) {
        float t = static_cast<float>(i) / (nbControlPoints - 1);
        glm::vec3 point = (1 - t) * startPoint + t * endPoint;
        controlPoints.push_back(point);
    }
}

void BezierCurve::setResolution(unsigned int new_res) {
    resolution = new_res;
}

void BezierCurve::setHeightData(const std::vector<float>& heightData) {
    m_heightData = heightData;
}