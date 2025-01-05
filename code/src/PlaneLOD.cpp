#include "PlaneLOD.hpp"
#include <shader.hpp>
#include <stb_image.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


PlaneLOD::PlaneLOD(float new_size, unsigned int new_resolution, Camera* cam) {
    size = new_size;
    resolution = new_resolution;
    camera_plan = cam;
    displayWire = false;
    displayPoint = false;
    showNormals = false;
    heightScale = 10.0f;
    maxLodDistance = 100.0f;
    morphFactor = 0.2f; 
    morphDistance = maxLodDistance * morphFactor;

    color = glm::vec3(1.f, 1.f, 1.f);

    createPlaneVAO();

    m_shaderProgram = LoadShadersV2("../shaders/vertex_shader.glsl", "../shaders/fragment_shader.glsl", "../tesselation_shaders/tess_control_shader.glsl", "../tesselation_shaders/tess_eval_shader.glsl", nullptr);
    m_normalShaderProgram = LoadShaders("../shaders/normal_vertex_shader.glsl", "../shaders/normal_fragment_shader.glsl", "../shaders/normal_geometry_shader.glsl");

    m_grassTextureID = loadTexture("../textures/grass.png");
    m_rockTextureID = loadTexture("../textures/rock.png");
    m_snowTextureID = loadTexture("../textures/snowrocks.png");

    grassLimit = 0.3f;
    rockLimit = 0.7f;

    initLight();

}

PlaneLOD::~PlaneLOD() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &m_textureID);
}

void PlaneLOD::draw() {

    glUseProgram(m_shaderProgram);

    glPatchParameteri(GL_PATCH_VERTICES, 3); // Chaque patch contient 3 sommets

    // Matrices de transformation
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    const glm::mat4& viewMatrix = camera_plan->getViewMatrix();
    const glm::mat4& projectionMatrix = camera_plan->getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    // Hauteur et texture du heightMap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_heightMapID);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "heightMap"), 0);

    // Lier les textures pour herbe, rocher et neige
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_grassTextureID);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "grassTexture"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_rockTextureID);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "rockTexture"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_snowTextureID);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "snowTexture"), 3);

    glUniform1f(glGetUniformLocation(m_shaderProgram, "heightScale"), heightScale);

    // Transmettre la position de la caméra
    glm::vec3 cameraPos = camera_plan->getPosition(); // Récupérer la position de la caméra
    glUniform3f(glGetUniformLocation(m_shaderProgram, "cameraPosition"), cameraPos.x, cameraPos.y, cameraPos.z);

    // Transmettre la distance maximale du LOD
    glUniform1f(glGetUniformLocation(m_shaderProgram, "lodDistance"), maxLodDistance);

    // == TENTATIVE DE REGLER PROBLEME DE TRANSITION DE LOD == //

    // Récupérer les paramètres de la caméra
    float nearPlane = camera_plan->getNear();
    float farPlane = camera_plan->getFar();
    float tessDistance = (farPlane - nearPlane) / 3.0f; 

    // Transmettre les uniformes
    glUniform1f(glGetUniformLocation(m_shaderProgram, "u_Near"), -nearPlane);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "u_Far"), -farPlane);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "u_TessDistance"), tessDistance);

    // Transmettre la distance de morphing
    morphDistance = maxLodDistance * morphFactor;
    glUniform1f(glGetUniformLocation(m_shaderProgram, "morphDistance"), morphDistance);

    glm::vec3 planeCenter(0.0f, 0.0f, 0.0f);
    float distance = glm::distance(cameraPos, planeCenter);

    // Mise à jour des facteurs de mélange
    blendFactors[0] = glm::clamp(1.0f - distance / (maxLodDistance / 3.0f), 0.0f, 1.0f);
    blendFactors[1] = glm::clamp(1.0f - glm::abs(distance - maxLodDistance / 3.0f) / (maxLodDistance / 3.0f), 0.0f, 1.0f);
    blendFactors[2] = glm::clamp((distance - 2.0f * maxLodDistance / 3.0f) / (maxLodDistance / 3.0f), 0.0f, 1.0f);

    for (int i = 0; i < 3; i++) {
        glUniform1f(glGetUniformLocation(m_shaderProgram, "blendFactor"), blendFactors[i]);
    }

    // == //

    // Transmettre la resolution du terrain
    glUniform1i(glGetUniformLocation(m_shaderProgram, "resolution"), resolution);

    //Transmettre les limites de mon truc
    glUniform1f(glGetUniformLocation(m_shaderProgram, "grassLimit"), grassLimit);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "rockLimit"), rockLimit);

    // Les lights
    glUniform3f(glGetUniformLocation(m_shaderProgram, "lightDirection"), lightDirection.r, lightDirection.g, lightDirection.b);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "lightColor"), lightColor.r, lightColor.g, lightColor.b);
    glUniform3f(glGetUniformLocation(m_shaderProgram, "ambientColor"), ambientColor.r, ambientColor.g, ambientColor.b);

    if(displayWire)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if(displayPoint)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Dessiner les triangles
    glBindVertexArray(VAO);
    glDrawElements(GL_PATCHES, m_indexCount, GL_UNSIGNED_INT, 0); 
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// == DEBUG : Affiche normales == //
void PlaneLOD::drawNormals(){
    glUseProgram(m_normalShaderProgram);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    const glm::mat4& viewMatrix = camera_plan->getViewMatrix();
    const glm::mat4& projectionMatrix = camera_plan->getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_normalShaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_normalShaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_normalShaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_heightMapID);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "heightMap"), 0);

    glUniform1f(glGetUniformLocation(m_shaderProgram, "heightScale"), heightScale);

    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, indices.size() / 3);
    glBindVertexArray(0);
}


void PlaneLOD::update(){
    draw();
    updateLightRotation();

    if(showNormals){
        drawNormals();
    }
}


void PlaneLOD::createPlaneVAO() {
    float halfSize = size / 2.0f;
    float step = size / resolution;

    for (unsigned int z = 0; z <= resolution; ++z) {
        for (unsigned int x = 0; x <= resolution; ++x) {
            float xPos = -halfSize + x * step;
            float zPos = -halfSize + z * step;

            vertices.push_back(xPos);
            vertices.push_back(0.0f);
            vertices.push_back(zPos);

            normals.push_back(0.0f);
            normals.push_back(1.0f);
            normals.push_back(0.0f);

            uvs.push_back(static_cast<float>(x) / resolution);
            uvs.push_back(static_cast<float>(z) / resolution);
        }
    }

    for (unsigned int z = 0; z < resolution; ++z) {
        for (unsigned int x = 0; x < resolution; ++x) {
            unsigned int topLeft = (z + 1) * (resolution + 1) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = z * (resolution + 1) + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topLeft);

            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
            indices.push_back(topRight);
        }
    }

    m_indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &UVBO);
    glGenBuffers(1, &NBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, UVBO);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

GLuint PlaneLOD::loadTexture(const std::string &path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 3);

    if (!data)
    {
        stbi_image_free(data);
        throw std::runtime_error("Failed to load texture: " + path);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return texture;
}

void PlaneLOD::showImGuiInterface() {
    static float prevSize = size;
    static int prevResolution = resolution;

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Info Plan")) {

        if (ImGui::SliderFloat("Size", &size, 10.0f, 1000.0f)) {
            if (size != prevSize) {
                recreatePlane();
                prevSize = size;
            }
        }
        if (ImGui::SliderInt("Resolution", &resolution, 1, 200)) {
            if (resolution != prevResolution) {
                recreatePlane();
                prevResolution = resolution;
            }
        }
        ImGui::SliderFloat("Scale hauteur", &heightScale, 1.0f, 100.0f);
        ImGui::SliderFloat("LOD Distance", &maxLodDistance, 10.0f, 500.0f);
        ImGui::SliderFloat("Morph Factor", &morphFactor, 0.01f, 1.0f);
        ImGui::SliderFloat("Grass Limit", &grassLimit, -1.0f, 1.0f);
        ImGui::SliderFloat("Rock Limit", &rockLimit, -1.0f, 1.0f);

        ImGui::Separator();
        ImGui::Text(" === Modes d'affichage ===");
        if(ImGui::Checkbox("Afficher les triangles", &displayWire)){
            displayPoint = false;
        }
        ImGui::SameLine();
        if(ImGui::Checkbox("Afficher les points", &displayPoint)){
            displayWire = false;
        }
        ImGui::Checkbox("Afficher les normales", &showNormals);

        ImGui::Text(" === Lumière ===");
        ImGui::SliderFloat("Angle rotation", &lightRotationAngle, -180.0f, 180.0f);

        if (ImGui::Button("Reload Shaders")) {
            reloadShaders();
            recreatePlane();
        }
    }
    ImGui::End();
}

void PlaneLOD::recreatePlane() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &UVBO);
    glDeleteBuffers(1, &NBO);
    

    vertices.clear();
    uvs.clear();
    indices.clear();
    normals.clear();

    createPlaneVAO();
}

void PlaneLOD::reloadShaders() {
    glDeleteProgram(m_shaderProgram);
    glDeleteProgram(m_normalShaderProgram);

    m_shaderProgram = LoadShadersV2(
        "../shaders/vertex_shader.glsl", 
        "../shaders/fragment_shader.glsl", 
        "../tesselation_shaders/tess_control_shader.glsl", 
        "../tesselation_shaders/tess_eval_shader.glsl", 
        nullptr
    );

    m_normalShaderProgram = LoadShaders(
        "../shaders/normal_vertex_shader.glsl", 
        "../shaders/normal_fragment_shader.glsl", 
        "../shaders/normal_geometry_shader.glsl"
    );

    std::cout << "Shaders reloaded successfully!" << std::endl;
}


void PlaneLOD::initLight(){
    lightDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

    lightRotationAngle = 100.0f;
}

void PlaneLOD::updateLightRotation(){
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));

    lightDirection = glm::vec3(rotationMatrix * glm::vec4(glm::vec3(0.0f, -1.0f, -1.0f), 0.0f));
}

int PlaneLOD::getResolution() const{
    return resolution;
}

float PlaneLOD::getSize() const{
    return size;
}

float PlaneLOD::getHeightScale() const{
    return heightScale;
}

void PlaneLOD::setHeightMap(GLuint heightMapID){
    m_heightMapID = heightMapID;
    setHeight();
}

int PlaneLOD::getLodDistance() const{
    return maxLodDistance;
}

int PlaneLOD::getMorphFactor() const{
    return morphFactor;
}

int PlaneLOD::getMorphDistance() const{
    return morphDistance;
}
void PlaneLOD::setHeight() {
    if (m_heightMapID == 0) {
        std::cerr << "Height map ID invalide!" << std::endl;
        return;
    }

    // Synchronisation GPU-CPU
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    m_heightData.resize(resolution * resolution);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, m_heightMapID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, m_heightData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

const std::vector<float>& PlaneLOD::getHeightData() const {
    return m_heightData;
}

Camera* PlaneLOD::getCamera() const {
    return camera_plan;
}

float PlaneLOD::getHeightDataAt(float x, float z) const {
    if (m_heightData.empty() || resolution <= 0) {
        std::cerr << "Height data pas disponible!" << std::endl;
        return 0.0f;
    }

    float normalizedX = glm::clamp(x / size, 0.0f, 1.0f);
    float normalizedZ = glm::clamp(z / size, 0.0f, 1.0f);

    int ix = static_cast<int>(normalizedX * resolution);
    int iz = static_cast<int>(normalizedZ * resolution);

    return m_heightData[iz * resolution + ix];
}

const std::vector<float>& PlaneLOD::getVertices() const {
    return vertices;
}

