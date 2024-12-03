#include "Plane.hpp"
#include <shader.hpp>
#include <stb_image.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


Plane::Plane(float new_size, unsigned int new_resolution, Camera* cam) {
    size = new_size;
    resolution = new_resolution;
    camera_plan = cam;
    isWireframe = false;
    showNormals = false;
    heightScale = 10.0f;
    maxLodDistance = 100.0f;
    color = glm::vec3(1.f, 1.f, 1.f);

    createPlaneVAO();

    m_shaderProgram = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl", "lod_geometry_shader.glsl");
    m_normalShaderProgram = LoadShaders("normal_vertex_shader.glsl", "normal_fragment_shader.glsl", "normal_geometry_shader.glsl");
    m_lodShaderProgram = LoadShaders("lod_vertex_shader.glsl", "lod_fragment_shader.glsl");

    m_grassTextureID = loadTexture("../textures/grass.png");
    m_rockTextureID = loadTexture("../textures/rock.png");
    m_snowTextureID = loadTexture("../textures/snowrocks.png");

    grassLimit = 0.4f;
    rockLimit = 0.7f;

    m_ColorID = glGetUniformLocation(m_shaderProgram, "color_Mesh");

    initLodFBO();
}

Plane::~Plane() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &m_textureID);
}

void Plane::draw() {

    glUseProgram(m_shaderProgram);

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
    // MARCHE PAS
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

    //Transmettre les limites de mon truc
    glUniform1f(glGetUniformLocation(m_shaderProgram, "grassLimit"), grassLimit);
    glUniform1f(glGetUniformLocation(m_shaderProgram, "rockLimit"), rockLimit);

    // Couleur du maillage
    glUniform3f(m_ColorID, color.x, color.y, color.z);

    // Mode fil de fer ou rempli
    glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);

    // Dessiner les triangles
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Réinitialiser le mode de polygone
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Plane::drawNormals(){
    glUseProgram(m_normalShaderProgram);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    const glm::mat4& viewMatrix = camera_plan->getViewMatrix();
    const glm::mat4& projectionMatrix = camera_plan->getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_normalShaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_normalShaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_normalShaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
    glBindVertexArray(0);
}


void Plane::update(){
    showImGuiInterface();
    renderLod();
    showImGuiLOD();
    draw();

    if(showNormals){
        drawNormals();
    }
}


void Plane::createPlaneVAO() {
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

void Plane::initLodFBO() {
    glGenFramebuffers(1, &lodFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lodFBO);

    glGenTextures(1, &lodTexture);
    glBindTexture(GL_TEXTURE_2D, lodTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lodTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error while creating FBO" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Plane::renderLod() {
    glBindFramebuffer(GL_FRAMEBUFFER, lodFBO);
    glViewport(0, 0, 1024, 1024);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_lodShaderProgram);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    const glm::mat4& viewMatrix = camera_plan->getViewMatrix();
    const glm::mat4& projectionMatrix = camera_plan->getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_lodShaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_lodShaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_lodShaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniform1f(glGetUniformLocation(m_lodShaderProgram, "lodDistance"), maxLodDistance); 
    glUniform1f(glGetUniformLocation(m_lodShaderProgram, "heightScale"), heightScale);

    glBindTexture(GL_TEXTURE_2D, m_heightMapID);
    glUniform1i(glGetUniformLocation(m_lodShaderProgram, "heightMap"), 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// GLuint Plane::loadTexture(const std::string& texturePath) {
//     GLuint textureID;
//     glGenTextures(1, &textureID);

//     int width, height, nrChannels;
//     unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, STBI_rgb);

//     if (data) {
//         GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
//         glBindTexture(GL_TEXTURE_2D, textureID);
//         glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//         glGenerateMipmap(GL_TEXTURE_2D);

//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


//         std::cout << "Texture loaded: " << texturePath
//               << " (" << width << "x" << height << ", " << nrChannels << " channels)" << std::endl;
//     } else {
//         std::cerr << "Failed to load texture: " << texturePath << std::endl;
//         return 0;
//     }

//     stbi_image_free(data);
//     return textureID;
// }

GLuint Plane::loadTexture(const std::string &path)
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

void Plane::showImGuiInterface() {
    static float prevSize = size;
    static int prevResolution = resolution;

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Info Plan")) {
        ImGui::Text("Couleur(%f,%f,%f)", color.x, color.y, color.z);
		ImGui::SliderFloat("R", &color.x, 0.0f, 1.0f);
		ImGui::SliderFloat("G", &color.y, 0.0f, 1.0f);
		ImGui::SliderFloat("B", &color.z, 0.0f, 1.0f);

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
        if (ImGui::SliderFloat("LOD Distance", &maxLodDistance, 10.0f, 500.0f)) {
            renderLod();
        }
        ImGui::SliderFloat("Grass Limit", &grassLimit, -1.0f, 1.0f);
        ImGui::SliderFloat("Rock Limit", &rockLimit, -1.0f, 1.0f);

        ImGui::Checkbox("Mode d'affichage", &isWireframe);
        ImGui::Checkbox("Afficher les normales", &showNormals);
    }
    ImGui::End();
}

void Plane::showImGuiLOD() {
    if (ImGui::Begin("LOD Preview")) {
        ImGui::Image((void*)(intptr_t)lodTexture, ImVec2(300, 300));
    }
    ImGui::End();
}

void Plane::recreatePlane() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &UVBO);
    glDeleteBuffers(1, &NBO);
    

    vertices.clear();
    uvs.clear();
    indices.clear();

    createPlaneVAO();
}

int Plane::getResolution(){
    return resolution;
}

void Plane::setHeightMap(GLuint heightMapID){
    m_heightMapID = heightMapID;
}