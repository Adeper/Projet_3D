#include "Curve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <shader.hpp>
#include <iostream>
#include <stb_image.h>

Curve::Curve(PlaneLOD* terrain)
    : terrain(terrain), VAO(0), VBO(0), curveType(BEZIER) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    shaderProgram = LoadShaders("../shaders/curve_vertex_shader.glsl", "../shaders/curve_fragment_shader.glsl");
    color = glm::vec3(1.0f, 0.0f, 0.0f);
    useTexture = false;
    heightOffset = 0.1f;
    initControlPointsFromTerrain();
}

Curve::~Curve() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

// void Curve::initControlPoints(const glm::vec3& startPoint, const glm::vec3& endPoint, int nbControlPoints) {
//     controlPoints.clear();
//     for (int i = 0; i < nbControlPoints; ++i) {
//         float t = static_cast<float>(i) / (nbControlPoints - 1);
//         glm::vec3 point = (1 - t) * startPoint + t * endPoint;
//         controlPoints.push_back(point);
//     }
// }

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
    initControlPointsFromTerrain();

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

    if (useTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "curveTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 1);
    } else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);
        glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &color[0]);
    }

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
        point.y = terrain->getHeightDataAt(point.x, point.z) * terrain->getHeightScale() + heightOffset;
    }
}

void Curve::showImGuiInterface() {
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Info courbe")) {
        ImGui::Text("Type de courbe");
        ImGui::RadioButton("Bézier", reinterpret_cast<int*>(&curveType), BEZIER);
        ImGui::RadioButton("Catmull-Rom", reinterpret_cast<int*>(&curveType), CATMULL_ROM);
        ImGui::RadioButton("Approximation", reinterpret_cast<int*>(&curveType), APPROXIMATION);

        ImGui::Separator();
        ImGui::SliderFloat("Décalage hauteur", &heightOffset, 0.0f, 1.0f);

        ImGui::Separator();
        ImGui::ColorEdit3("Couleur", &color[0]);
        ImGui::Checkbox("Charger une texture", &useTexture);
        if (useTexture) {
            try {
                loadTexture(std::string("../textures/road.jpg"));
            } catch (const std::exception& e) {
                std::cerr << "Erreur : " << e.what() << std::endl;
            }
        }

        if (ImGui::Button("Reload Shaders")) {
            reloadShaders();
        }

    }
    ImGui::End();
}

void Curve::loadTexture(const std::string &path) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        stbi_image_free(data);
        throw std::runtime_error("Erreur lors du chargement des textures: " + path);
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    useTexture = true;

}

void Curve::reloadShaders() {
    glDeleteProgram(shaderProgram);
    shaderProgram = LoadShaders("../shaders/curve_vertex_shader.glsl", "../shaders/curve_fragment_shader.glsl");
    update();
}

void Curve::initControlPointsFromTerrain() {
    controlPoints.clear();
    const std::vector<float>& terrainVertices = terrain->getVertices();

    //utiliser les sommets d'une ligne (fixer z et itérer sur x)
    int terrainResolution = terrain->getResolution();
    for (int x = 0; x < terrainResolution; ++x) {
        int vertexIndex = (x + (terrainResolution / 2) * terrainResolution) * 3; // Indice dans le tableau
        if (vertexIndex + 2 < terrainVertices.size()) {
            glm::vec3 point(
                terrainVertices[vertexIndex],     // x
                terrainVertices[vertexIndex + 1], // y
                terrainVertices[vertexIndex + 2]  // z
            );
            controlPoints.push_back(point);
        }
    }
}


