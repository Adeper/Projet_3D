#include "Curve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <shader.hpp>
#include <iostream>

Curve::Curve(PlaneLOD* terrain)
    : terrain(terrain), VAO(0), VBO(0), curveType(BEZIER) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    shaderProgram = LoadShaders("../shaders/curve_vertex_shader.glsl", "../shaders/curve_fragment_shader.glsl");
}

Curve::~Curve() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Curve::initControlPoints(const glm::vec3& startPoint, const glm::vec3& endPoint, int nbControlPoints) {
    controlPoints.clear();
    for (int i = 0; i < nbControlPoints; ++i) {
        float t = static_cast<float>(i) / (nbControlPoints - 1);
        glm::vec3 point = (1 - t) * startPoint + t * endPoint;
        controlPoints.push_back(point);
    }
}

void Curve::setCurveType(CurveType type) {
    curveType = type;
}

void Curve::update() {
    curvePoints.clear();

    switch (curveType) {
        case BEZIER:
            computeBezierCurve();
            break;
        case CATMULL_ROM:
            computeCatmullRomCurve();
            break;
        case APPROXIMATION:
            computeApproximationCurve(controlPoints.front(), controlPoints.back());
            break;
    }

    applyHeightToCurve();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Curve::draw() {
    glUseProgram(shaderProgram);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    const glm::mat4& viewMatrix = terrain->getCamera()->getViewMatrix();
    const glm::mat4& projMatrix = terrain->getCamera()->getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projMatrix[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());
    glBindVertexArray(0);

    glUseProgram(0);
}

void Curve::computeBezierCurve() {
    for (int i = 0; i <= terrain->getResolution(); ++i) {
        float t = static_cast<float>(i) / terrain->getResolution();
        curvePoints.push_back(deCasteljau(t));
    }
}

void Curve::computeCatmullRomCurve() {
    for (size_t i = 0; i < controlPoints.size() - 3; ++i) {
        for (int j = 0; j <= terrain->getResolution(); ++j) {
            float t = static_cast<float>(j) / terrain->getResolution();
            curvePoints.push_back(catmullRom(t, controlPoints[i], controlPoints[i + 1], controlPoints[i + 2], controlPoints[i + 3]));
        }
    }
}

void Curve::computeApproximationCurve(const glm::vec3& startPoint, const glm::vec3& endPoint) {
    for (int i = 0; i <= terrain->getResolution(); ++i) {
        float t = static_cast<float>(i) / terrain->getResolution();
        glm::vec3 point = (1 - t) * startPoint + t * endPoint;
        curvePoints.push_back(point);
    }
}

glm::vec3 Curve::deCasteljau(float t) const {
    std::vector<glm::vec3> points = controlPoints;
    while (points.size() > 1) {
        for (size_t i = 0; i < points.size() - 1; ++i) {
            points[i] = (1 - t) * points[i] + t * points[i + 1];
        }
        points.pop_back();
    }
    return points[0];
}

glm::vec3 Curve::catmullRom(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const {
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
}

void Curve::applyHeightToCurve() {
    for (auto& point : curvePoints) {
        point.y = terrain->getHeightDataAt(point.x, point.z) * terrain->getHeightScale();
    }
}
