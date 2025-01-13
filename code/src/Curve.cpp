#include "Curve.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <shader.hpp>
#include <iostream>
#include <stb_image.h>
#include <objloader.hpp>

#include <queue>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <functional>

float Node::stepSize = 2.0f;

Curve::Curve(PlaneLOD* terrain, Noise* noise)
    : terrain(terrain), noise(noise), VAO(0), VBO(0), curveType(CATMULL_ROM) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    shaderProgram = LoadShaders("../shaders/curve_vertex_shader.glsl", "../shaders/curve_fragment_shader.glsl");
    color = glm::vec3(1.0f, 0.0f, 0.0f);
    showControlPoints = false; 
    useTexture = false;
    showRoad = false;
    heightOffset = 0.f;
    curveWidth = 1.0f;
    startPoint = glm::vec3(0. -(terrain->getSize()/2.),0.,0. -(terrain->getSize()/2.));
    endPoint = glm::vec3(terrain->getSize()/2., 0., terrain->getSize()/2.);
    iterationGradiant = 10;
    nbControlPoints = 4;
    sphereLoaded = false;
    GLuint sphereVAO = 0;
    GLuint sphereVBO = 0;
    loadSphere("../data/sphere.off");
    loadTexture("../textures/route_pierre.jpg");
    initControlPoints();

    // pour A*
    heightWeight = 2.0f;
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

    //updateStartEndPoints();
    
    switch (curveType) {
        case CATMULL_ROM:
            updateControlPoints();
            adjustControlPoints();
            computeCatmullRomCurve();
            break;
        case ASTAR:
            computeAStar();
            break;
    }
    
    applyHeightToCurve();

    if(terrain->getUseTesselation()){
        heightOffset = 1.f;
    }else{
        heightOffset = 0.f;
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Curve::updateStartEndPoints(){
    float sizePlanLimit = terrain->getSize() / 2.;
    startPoint = glm::vec3(-sizePlanLimit, terrain->getHeightDataAt(-sizePlanLimit, -sizePlanLimit), -sizePlanLimit);
    endPoint = glm::vec3(sizePlanLimit, terrain->getHeightDataAt(sizePlanLimit, sizePlanLimit), sizePlanLimit);
}

void Curve::updateControlPoints() {
    if (controlPoints.empty()) {
        initControlPoints();
        return;
    }

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
            float sizePlanLimit = terrain->getSize() / 2.;

            float heightL = terrain->getHeightDataAt(glm::clamp(point.x - stepSize, -sizePlanLimit, sizePlanLimit), point.z);
            float heightR = terrain->getHeightDataAt(glm::clamp(point.x + stepSize, -sizePlanLimit, sizePlanLimit), point.z);
            float heightD = terrain->getHeightDataAt(point.x, glm::clamp(point.z - stepSize, -sizePlanLimit, sizePlanLimit));
            float heightU = terrain->getHeightDataAt(point.x, glm::clamp(point.z + stepSize, -sizePlanLimit, sizePlanLimit));

            // Gradient en X et Z
            float gradX = heightL - heightR;
            float gradZ = heightD - heightU;

            float newX = point.x + gradX;
            float newZ = point.z + gradZ;


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

    glUniform1i(glGetUniformLocation(shaderProgram, "showRoad"), showRoad);

    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &color[0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0, curvePoints.size());
    glBindVertexArray(0);

    drawControlPoints();
    drawRoad();

    glUseProgram(0);
}

void Curve::drawRoad() {
    if (!showRoad) return;

    glBindVertexArray(VAO);
    glUseProgram(shaderProgram);

    

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shaderProgram, "curveTexture"), 5);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrain->getHeightMap());
    glUniform1i(glGetUniformLocation(shaderProgram, "heightMap"), 0);

    for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
        glm::vec3 p1 = curvePoints[i];
        glm::vec3 p2 = curvePoints[i + 1];

        glm::vec3 direction = glm::normalize(glm::vec3(p2.x,0.,p2.z) - glm::vec3(p1.x,0.,p1.z));
        glm::vec3 perpendicular = glm::vec3(-direction.z, 0.0f, direction.x);

        float width = 1./10.;

        glm::vec3 bottomLeft = p1 - perpendicular * width;
        bottomLeft.y = getHeightForDrawAtCoord(bottomLeft.x, bottomLeft.z);
        if (fabs(bottomLeft.y - p1.y) > width) { // Limitation en hauteur
            if (bottomLeft.y > p1.y) {
                bottomLeft.y = p1.y + width;
            } else {
                bottomLeft.y = p1.y - width;
            }
        }

        glm::vec3 bottomRight = p1 + perpendicular * width;
        bottomRight.y = getHeightForDrawAtCoord(bottomRight.x, bottomRight.z);
        if (fabs(bottomRight.y - p1.y) > width) { // Limitation en hauteur
            if (bottomRight.y > p1.y) {
                bottomRight.y = p1.y + width;
            } else {
                bottomRight.y = p1.y - width;
            }
        }

        glm::vec3 topLeft = p2 - perpendicular * width;
        topLeft.y = getHeightForDrawAtCoord(topLeft.x, topLeft.z);
        if (fabs(topLeft.y - p2.y) > width) { // Limitation en hauteur
            if (topLeft.y > p2.y) {
                topLeft.y = p2.y + width;
            } else {
                topLeft.y = p2.y - width;
            }
        }

        glm::vec3 topRight = p2 + perpendicular * width;
        topRight.y = getHeightForDrawAtCoord(topRight.x, topRight.z);
        if (fabs(topRight.y - p2.y) > width) { // Limitation en hauteur
            if (topRight.y > p2.y) {
                topRight.y = p2.y + width;
            } else {
                topRight.y = p2.y - width;
            }
        }

        /*glm::vec3 bottomLeft = glm::vec3(p1.x - perpendicular.x * width, getHeightForDrawAtCoord(p1.x - perpendicular.x * width, p1.z - perpendicular.z * width), p1.z - perpendicular.z * width);
        glm::vec3 bottomRight = glm::vec3(p1.x + perpendicular.x * width, getHeightForDrawAtCoord(p1.x + perpendicular.x * width, p1.z + perpendicular.z * width), p1.z + perpendicular.z * width);
        glm::vec3 topLeft = glm::vec3(p2.x - perpendicular.x * width, getHeightForDrawAtCoord(p2.x - perpendicular.x * width, p2.z - perpendicular.z * width), p2.z - perpendicular.z * width);
        glm::vec3 topRight = glm::vec3(p2.x + perpendicular.x * width, getHeightForDrawAtCoord(p2.x + perpendicular.x * width, p2.z + perpendicular.z * width), p2.z + perpendicular.z * width);
*/
        glm::vec3 vertices[6] = {
            bottomLeft, bottomRight, topRight,
            bottomLeft, topRight, topLeft
        };

        float length = glm::distance(p2, p1);

        glm::vec2 uvs[6] = {
            glm::vec2(0.0f, 0.0f),        glm::vec2(1.0f, 0.0f),        glm::vec2(1.0f, length),
            glm::vec2(0.0f, 0.0f),        glm::vec2(1.0f, length),      glm::vec2(0.0f, length)
        };


        GLuint VBO, UVBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &UVBO);
        glBindBuffer(GL_ARRAY_BUFFER, UVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, UVBO);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &UVBO);
    }

    glBindVertexArray(0);
}


// catmullRom fonctions

void Curve::computeCatmullRomCurve() {
    for (size_t i = 0; i < controlPoints.size() - 3; ++i) {
        for (int j = 0; j <= terrain->getResolution(); ++j) {
            float t = static_cast<float>(j) / terrain->getResolution();
            curvePoints.push_back(catmullRom(t, controlPoints[i], controlPoints[i + 1], controlPoints[i + 2], controlPoints[i + 3]));
        }
    }
}

glm::vec3 Curve::catmullRom(float t, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) const {
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t);
}

// A* fonctions

float Curve::heuristic(const glm::vec3& current, const glm::vec3& goal) {
    // Somme la différence de hauteur entre current et goal par saut de stepSize
    float heuristicSum = 0.0f;
    glm::vec3 direction = glm::normalize(goal - current);
    glm::vec3 position = current;

    float stepSize_local = Node::getStepSize();
    float sizePlanLimit = terrain->getSize() / 2.0f;

    while (glm::distance(position, goal) > stepSize_local) {
        glm::vec3 nextPosition = position + direction * stepSize_local;

        if (std::abs(nextPosition.x) > sizePlanLimit || std::abs(nextPosition.z) > sizePlanLimit) {
            break;
        }

        nextPosition.y = terrain->getHeightDataAt(nextPosition.x, nextPosition.z);

        float horizontalDistance = glm::distance(glm::vec2(position.x, position.z), glm::vec2(nextPosition.x, nextPosition.z));
        float verticalDistance = std::abs(nextPosition.y - position.y) * heightWeight;

        heuristicSum += horizontalDistance + verticalDistance;

        position = nextPosition;
    }

    return heuristicSum;
}



// Spécialisation de std::hash pour glm::vec3
namespace std {
    template <>
    struct hash<glm::vec3> {
        std::size_t operator()(const glm::vec3& v) const {
            std::size_t h1 = std::hash<float>()(v.x);
            std::size_t h2 = std::hash<float>()(v.y);
            std::size_t h3 = std::hash<float>()(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

void Curve::computeAStar() {

    if (startPoint == endPoint) {
        curvePoints = {startPoint};
        return;
    }

    float sizePlanLimit = terrain->getSize() / 2.0f;
    float stepSize_local = Node::getStepSize();

    std::priority_queue<Node> openSet;
    std::unordered_map<glm::vec3, Node*, std::hash<glm::vec3>> nodes;

    Node* startNode = new Node(startPoint);
    startNode->gScore = 0.0f;
    startNode->hScore = heuristic(startPoint, endPoint);
    startNode->fScore = startNode->hScore;

    openSet.push(*startNode);
    nodes[startPoint] = startNode;

    Node* goalNode = nullptr;

    while (!openSet.empty()) {
        Node* current = new Node(openSet.top());
        openSet.pop();

        if (glm::distance(current->position, endPoint) < stepSize_local) {
            current->position = endPoint;
            goalNode = current;
            break;
        }

        std::vector<glm::vec3> neighbors = getNeighbors(current->position);

        for (const glm::vec3& neighborPos : neighbors) {
            if (std::abs(neighborPos.x) > sizePlanLimit || std::abs(neighborPos.z) > sizePlanLimit) {
                continue;
            }

            float tentative_gScore = current->gScore + glm::distance(current->position, neighborPos);

            if (nodes.find(neighborPos) == nodes.end() || tentative_gScore < nodes[neighborPos]->gScore) {
                Node* neighborNode = nodes[neighborPos];
                if (!neighborNode) {
                    neighborNode = new Node(neighborPos);
                    nodes[neighborPos] = neighborNode;
                }
                neighborNode->parent = current;
                neighborNode->gScore = tentative_gScore;
                neighborNode->hScore = heuristic(neighborPos, endPoint);
                neighborNode->fScore = neighborNode->gScore + neighborNode->hScore;

                openSet.push(*neighborNode);
            }
        }
    }

    if (goalNode) {
        curvePoints = reconstructPath(goalNode);
    } else {
        std::cerr << "Failed to find a path.\n";
    }

    for (auto& node : nodes) {
        delete node.second;
    }

}

std::vector<glm::vec3> Curve::getNeighbors(const glm::vec3& position) {
    std::vector<glm::vec3> neighbors;
    float stepSize_local = Node::getStepSize();

    for (float dx = -stepSize_local; dx <= stepSize_local; dx += stepSize_local) {
        for (float dz = -stepSize_local; dz <= stepSize_local; dz += stepSize_local) {
            if (dx == 0 && dz == 0) continue;
            glm::vec3 neighborPos = position + glm::vec3(dx, 0.0f, dz);
            neighborPos.y = terrain->getHeightDataAt(neighborPos.x, neighborPos.z);
            neighbors.push_back(neighborPos);
        }
    }

    return neighbors;
}

std::vector<glm::vec3> Curve::reconstructPath(Node* goalNode) {
    std::vector<glm::vec3> path;
    Node* current = goalNode;

    while (current != nullptr) {
        path.push_back(current->position);
        current = current->parent;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void Curve::applyHeightToCurve() {
    for (auto& point : curvePoints) {
        point.y = terrain->getHeightDataAt(point.x, point.z) * terrain->getHeightScale() + heightOffset;
    }
}

float Curve::getHeightForDrawAtCoord(const float x, const float z){
    return terrain->getHeightDataAt(x, z) * terrain->getHeightScale() + heightOffset;
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
        ImGui::RadioButton("Catmull-Rom", reinterpret_cast<int*>(&curveType), CATMULL_ROM);
        ImGui::RadioButton("A*", reinterpret_cast<int*>(&curveType), ASTAR);

        glm::vec2 startPoint2D = {startPoint.x, startPoint.z};
        glm::vec2 endPoint2D = {endPoint.x, endPoint.z};
        ImVec2 min(-terrain->getSize() / 2.0f, -terrain->getSize() / 2.0f);
        ImVec2 max(terrain->getSize() / 2.0f, terrain->getSize() / 2.0f);
        ImVec2 startPointImGui(startPoint2D.x, startPoint2D.y);
        ImVec2 endPointImGui(endPoint2D.x, endPoint2D.y);

        Draw2DSliderWithMultiplePoints("Position des points", startPointImGui, endPointImGui, ImVec2(150, 150), min, max);

        startPoint.x = startPointImGui.x;
        startPoint.z = startPointImGui.y;
        startPoint.y = terrain->getHeightDataAt(startPoint.x, startPoint.z);

        endPoint.x = endPointImGui.x;
        endPoint.z = endPointImGui.y;
        endPoint.y = terrain->getHeightDataAt(endPoint.x, endPoint.z);

        if(ImGui::Checkbox("Afficher le chemin", &showRoad)){
            showControlPoints = false;
        }

        if(curveType == CATMULL_ROM){
            // Parametres CATMULL_ROM
            ImGui::Separator();
            ImGui::Text("Parametres de la courbe");
            if(ImGui::Checkbox("Afficher les points de controle", &showControlPoints)){
                showRoad = false;
            }

            ImGui::SliderInt("iteration", &iterationGradiant, 1, 15);

            if(ImGui::SliderInt("Nb de points de controle", &nbControlPoints, 4, 25)){
                initControlPoints();
            }

        }else if(curveType == ASTAR){
            ImGui::SliderFloat("Poids de la hauteur", &heightWeight, 1.f, 20.0f);
        }

        ImGui::Separator();
        ImGui::ColorEdit3("Couleur", &color[0]);
        

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

float Curve::ImVec2DistanceSqr(const ImVec2& a, const ImVec2& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

void Curve::Draw2DSliderWithMultiplePoints(const char* label, ImVec2& point1, ImVec2& point2, const ImVec2& size, const ImVec2& min, const ImVec2& max) {
    ImGui::Text("%s", label);

    // Position et taille du carré
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 squareSize = size;

    // Dessiner le carré
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRect(cursorPos, ImVec2(cursorPos.x + squareSize.x, cursorPos.y + squareSize.y), IM_COL32(255, 255, 255, 255));

    // Dessiner les points
    auto drawPoint = [&](ImVec2& point, ImU32 color, float radius) {
        ImVec2 normalizedPoint(
            (point.x - min.x) / (max.x - min.x),
            (point.y - min.y) / (max.y - min.y)
        );

        ImVec2 pointPos(
            cursorPos.x + normalizedPoint.x * squareSize.x,
            cursorPos.y + normalizedPoint.y * squareSize.y
        );

        drawList->AddCircleFilled(pointPos, radius, color);
    };

    const float pointRadius = 7.0f; // Taille visuelle des points
    const float selectionRadius = 15.0f; // Zone de tolérance pour la sélection

    drawPoint(point1, IM_COL32(255, 0, 0, 255), pointRadius); // Point 1 en rouge
    drawPoint(point2, IM_COL32(0, 0, 255, 255), pointRadius); // Point 2 en bleu

    // Rendre la zone interactive
    ImGui::InvisibleButton(label, squareSize);
    if (ImGui::IsItemActive()) {
        ImVec2 mousePos = ImGui::GetMousePos();

        // Calculer les positions des points sur l'écran
        ImVec2 point1ScreenPos(
            cursorPos.x + ((point1.x - min.x) / (max.x - min.x)) * squareSize.x,
            cursorPos.y + ((point1.y - min.y) / (max.y - min.y)) * squareSize.y
        );

        ImVec2 point2ScreenPos(
            cursorPos.x + ((point2.x - min.x) / (max.x - min.x)) * squareSize.x,
            cursorPos.y + ((point2.y - min.y) / (max.y - min.y)) * squareSize.y
        );

        // Vérifier si la souris est proche d'un point
        bool nearPoint1 = ImVec2DistanceSqr(mousePos, point1ScreenPos) < selectionRadius * selectionRadius;
        bool nearPoint2 = ImVec2DistanceSqr(mousePos, point2ScreenPos) < selectionRadius * selectionRadius;

        // Mapper la position de la souris vers les coordonnées [min, max]
        ImVec2 clampedPos = ImVec2(
            std::max(cursorPos.x, std::min(cursorPos.x + squareSize.x, mousePos.x)),
            std::max(cursorPos.y, std::min(cursorPos.y + squareSize.y, mousePos.y))
        );

        ImVec2 normalizedMousePos(
            (clampedPos.x - cursorPos.x) / squareSize.x,
            (clampedPos.y - cursorPos.y) / squareSize.y
        );

        ImVec2 mappedMousePos(
            min.x + normalizedMousePos.x * (max.x - min.x),
            min.y + normalizedMousePos.y * (max.y - min.y)
        );

        // Déplacer le point sélectionné
        if (nearPoint1) {
            point1 = mappedMousePos;
        } else if (nearPoint2) {
            point2 = mappedMousePos;
        }
    }
}

void Curve::loadTexture(const std::string &path) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "Erreur lors du chargement de l'image : " << path << std::endl;
        throw std::runtime_error("Erreur lors du chargement des textures: " + path);
    }

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else {
        stbi_image_free(data);
        throw std::runtime_error("Format d'image non supporté: " + path);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::cout << "Texture chargée avec succès : " << path << " ("
              << width << "x" << height << ", " << nrChannels << " canaux)" << std::endl;

    useTexture = true;
}


void Curve::reloadShaders() {
    glDeleteProgram(shaderProgram);
    shaderProgram = LoadShaders("../shaders/curve_vertex_shader.glsl", "../shaders/curve_fragment_shader.glsl");
    update();
}