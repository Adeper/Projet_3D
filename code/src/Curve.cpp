#include "Curve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <shader.hpp>
#include <iostream>
#include <stb_image.h>
#include <objloader.hpp>

Curve::Curve(PlaneLOD* terrain, Noise* noise)
    : terrain(terrain), noise(noise), VAO(0), VBO(0), curveType(BEZIER) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    shaderProgram = LoadShaders("../shaders/curve_vertex_shader.glsl", "../shaders/curve_fragment_shader.glsl");
    color = glm::vec3(1.0f, 0.0f, 0.0f);
    showControlPoints = false; 
    useTexture = false;
    heightOffset = 0.1f;
    curveWidth = 1.0f;
    startPoint = glm::vec3(0. -(terrain->getSize()/2.),0.,0. -(terrain->getSize()/2.));
    endPoint = glm::vec3(terrain->getSize()/2., 0., terrain->getSize()/2.);
    iterationGradiant = 10;
    nbControlPoints = 4;
    sphereLoaded = false;
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    loadSphere("../data/sphere.off");
    initControlPoints();
}

Curve::~Curve() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void Curve::setCurveType(CurveType type) {
    curveType = type;
}

void Curve::initControlPoints() {
    if(nbControlPoints < 4){
        std::cerr << "Nombre de points de controle trop petit (doit etre >= 4)" << std::endl;
        return;
    }

    controlPoints.clear();

    controlPoints.push_back(startPoint);
    controlPoints.push_back(startPoint);

    // Calcul et ajout des points intermédiaires
    for (int i = 2; i < nbControlPoints; ++i) {
        float t = static_cast<float>(i-1) / (nbControlPoints-1);
        glm::vec3 interpolatedPoint = glm::mix(startPoint, endPoint, t);
        controlPoints.push_back(interpolatedPoint);
    }

    controlPoints.push_back(endPoint);
    controlPoints.push_back(endPoint);
}

void Curve::update() {
    curvePoints.clear();
    updateControlPoints();
    adjustControlPoints();

/*
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
    */

    computeCatmullRomCurve();

    applyHeightToCurve();

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Curve::updateControlPoints() {
    if (controlPoints.empty()) {
        initControlPoints();
        return;
    }

    startPoint = glm::vec3(0. -(terrain->getSize()/2.),0.,0. -(terrain->getSize()/2.));
    endPoint = glm::vec3(terrain->getSize()/2., 0., terrain->getSize()/2.);

    controlPoints[0] = startPoint;
    controlPoints[1] = startPoint;

    for (int i = 2; i < nbControlPoints; ++i) {
        float t = static_cast<float>(i-1) / (nbControlPoints-1);
        controlPoints[i] = glm::mix(startPoint, endPoint, t);
    }

    controlPoints[controlPoints.size()-2] = endPoint;
    controlPoints[controlPoints.size()-1] = endPoint;
}

void Curve::adjustControlPoints() {
    for (int iter = 0; iter < iterationGradiant; ++iter) {
        for (size_t i = 2; i < controlPoints.size() - 2; ++i) {
            glm::vec3& point = controlPoints[i];


            float stepSize = terrain->getSize() / 10.;

            float heightL = terrain->getHeightDataAt(point.x - stepSize, point.z);
            float heightR = terrain->getHeightDataAt(point.x + stepSize, point.z);
            float heightD = terrain->getHeightDataAt(point.x, point.z - stepSize);
            float heightU = terrain->getHeightDataAt(point.x, point.z + stepSize);

            // Gradient en X et Z
            float gradX = heightL - heightR;
            float gradZ = heightD - heightU;

            float newX = point.x + gradX;
            float newZ = point.z + gradZ;

            float sizePlanLimit = terrain->getSize() / 2.;

            point.x = glm::clamp(newX, -sizePlanLimit, sizePlanLimit);
            point.z = glm::clamp(newZ, -sizePlanLimit, sizePlanLimit);
        }
    }
}

void Curve::loadSphere(const std::string& filePath) {
    // Charger la sphère depuis le fichier OFF
    if (!loadOFF(filePath, sphereVertices, sphereFaces)) {
        std::cerr << "Erreur lors du chargement du fichier OFF pour la sphère : " << filePath << std::endl;
        return;
    }

    // Générer VAO et VBO pour la sphère
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(glm::vec3), sphereVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    sphereLoaded = true;
}

void Curve::drawControlPoints() {
    if (!showControlPoints || !sphereLoaded) return;

    glUseProgram(shaderProgram);

    for (const auto& point : controlPoints) {
        glm::mat4 modelMatrix = glm::translate( glm::mat4(1.0f), glm::vec3(
                                                                            point.x,
                                                                            terrain->getHeightDataAt(point.x, point.z) * terrain->getHeightScale() + heightOffset,
                                                                            point.z
                                                                        ));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f)); // Ajustez la taille des sphères

        const glm::mat4& viewMatrix = terrain->getCamera()->getViewMatrix();
        const glm::mat4& projMatrix = terrain->getCamera()->getProjectionMatrix();

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projMatrix[0][0]);

        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertices.size());
        glBindVertexArray(0);
    }

    glUseProgram(0);
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

    drawControlPoints();

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

void Curve::applyNoiseToCurve() {
    if (!noise) return;

    GLuint noiseTexture = noise->getTextureNoise();
    int noiseResolution = noise->getResolution();

    glBindTexture(GL_TEXTURE_2D, noiseTexture);

    // Lecture des données de la texture de bruit dans le CPU (si nécessaire)
    std::vector<float> noiseData(noiseResolution * noiseResolution);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, noiseData.data());

    // Appliquer le bruit à chaque point de la courbe
    for (auto& point : curvePoints) {
        float u = (point.x + 10.0f) / 20.0f; // Normalisation entre 0 et 1 (si -10 <= x <= 10)
        float v = (point.z + 10.0f) / 20.0f;

        // Convertir en coordonnées texture
        int x = static_cast<int>(u * (noiseResolution - 1));
        int y = static_cast<int>(v * (noiseResolution - 1));

        // Index dans les données de la texture
        int index = y * noiseResolution + x;

        // Perturbation par le bruit
        float noiseValue = noiseData[index];
        point.y += noiseValue * 0.5f; // Ajuster l'échelle du bruit si nécessaire
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void Curve::showImGuiInterface() {
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Info courbe")) {
        ImGui::Text("Type de courbe");
        ImGui::RadioButton("Bézier", reinterpret_cast<int*>(&curveType), BEZIER);
        ImGui::RadioButton("Catmull-Rom", reinterpret_cast<int*>(&curveType), CATMULL_ROM);
        ImGui::RadioButton("Approximation", reinterpret_cast<int*>(&curveType), APPROXIMATION);

        ImGui::Separator();
        ImGui::SliderFloat("Décalage hauteur", &heightOffset, 0.0f, 1.0f);

        ImGui::Separator();
        ImGui::Checkbox("Afficher les points de controle", &showControlPoints);

        ImGui::Separator();
        ImGui::SliderInt("iteration", &iterationGradiant, 1, 15);

        ImGui::Separator();
        if(ImGui::SliderInt("Nb de points de controle", &nbControlPoints, 4, 25)){
            initControlPoints();
        }

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

        ImGui::Separator();
        // DEBUG
        // if (ImGui::CollapsingHeader("Points de Contrôle")) {
        //     for (size_t i = 0; i < controlPoints.size(); ++i) {
        //         ImGui::SliderFloat3(("Point " + std::to_string(i)).c_str(), &controlPoints[i].x, -10.0f, 10.0f);
        //     }

        //     if (ImGui::Button("Ajouter un Point")) {
        //         controlPoints.push_back(controlPoints.back() + glm::vec3(1.0f, 0.0f, 0.0f));
        //     }

        //     if (controlPoints.size() > 2 && ImGui::Button("Supprimer un Point")) {
        //         controlPoints.pop_back();
        //     }
        // }

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

void Curve::addPointForWidth() {
    std::vector<glm::vec3> widenedCurvePoints;

    for (size_t i = 0; i < curvePoints.size(); ++i) {
        glm::vec3 currentPoint = curvePoints[i];
        glm::vec3 nextPoint = (i < curvePoints.size() - 1) ? curvePoints[i + 1] : curvePoints[i - 1];
        glm::vec3 tangent = glm::normalize(nextPoint - currentPoint);
        glm::vec3 normal = glm::normalize(glm::cross(tangent, glm::vec3(0.0f, 1.0f, 0.0f)));

        glm::vec3 leftPoint = currentPoint - normal * (curveWidth * 0.5f);
        glm::vec3 rightPoint = currentPoint + normal * (curveWidth * 0.5f);

        widenedCurvePoints.push_back(leftPoint);
        widenedCurvePoints.push_back(rightPoint);
    }

    curvePoints = widenedCurvePoints;
}



