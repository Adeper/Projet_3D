#include "Plane.hpp"

// Constructeur
Plane::Plane(int width, int height, float spacing)
    : width(width), height(height), spacing(spacing),
      vao(0), vboVertices(0), vboIndices(0), vboNormals(0), 
      translateX(0.0f), translateY(0.0f), translateZ(0.0f),
      rotateX(0.0f), rotateY(0.0f), rotateZ(0.0f) {}

// Destructeur
Plane::~Plane() {
    cleanUpBuffers();
}

// Initialisation
void Plane::init() {
    generateVertices();
    generateIndices();
    generateNormals();
    setupBuffers();
}

// Dessin
void Plane::draw() {
    glUseProgram(shaderProgram);

    // Uniformes pour les matrices
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // Matrices de transformation
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(translateX, translateY, translateZ));
    model = glm::rotate(model, glm::radians(rotateX), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotateY), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotateZ), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

// Mise à jour
void Plane::update(int newWidth, int newHeight) {
    if (newWidth != width || newHeight != height) {
        width = newWidth;
        height = newHeight;
        cleanUpBuffers();
        init();
    }
}

// Interface ImGui pour modifier les paramètres
void Plane::imguiInterface() {
    
    // Dimensions de la grille
    ImGui::Text("Paramètres de la grille:");
    static int newWidth = width;
    static int newHeight = height;

    if (ImGui::SliderInt("Longueur", &newWidth, 1, 100) || ImGui::SliderInt("Hauteur", &newHeight, 1, 100)) {
        update(newWidth, newHeight);
    }

    // Transformation : Translation
    ImGui::Text("Translation:");
    ImGui::SliderFloat("Translate X", &translateX, -10.0f, 10.0f);
    ImGui::SliderFloat("Translate Y", &translateY, -10.0f, 10.0f);
    ImGui::SliderFloat("Translate Z", &translateZ, -10.0f, 10.0f);

    // Transformation : Rotation
    ImGui::Text("Rotation:");
    ImGui::SliderFloat("Rotate X", &rotateX, 0.0f, 360.0f);
    ImGui::SliderFloat("Rotate Y", &rotateY, 0.0f, 360.0f);
    ImGui::SliderFloat("Rotate Z", &rotateZ, 0.0f, 360.0f);
}

// Génération des sommets
void Plane::generateVertices() {
    vertices.clear();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            vertices.push_back(i * spacing);
            vertices.push_back(0.0f);
            vertices.push_back(j * spacing);
        }
    }
}

// Génération des indices
void Plane::generateIndices() {
    indices.clear();
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            // Triangle 1
            indices.push_back(z * width + x);
            indices.push_back(z * width + (x + 1));
            indices.push_back((z + 1) * width + x);

            // Triangle 2
            indices.push_back((z + 1) * width + x);
            indices.push_back(z * width + (x + 1));
            indices.push_back((z + 1) * width + (x + 1));
        }
    }
}


// Génération des normales
void Plane::generateNormals() {
    normals.clear();
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            normals.push_back(0.0f);
            normals.push_back(1.0f);
            normals.push_back(0.0f);
        }
    }
}

void Plane::loadShaders(const char* vertexPath, const char* fragmentPath) {
    shaderProgram = LoadShaders(vertexPath, fragmentPath);
}

// Configuration des buffers
void Plane::setupBuffers() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vboIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

// Nettoyage des buffers
void Plane::cleanUpBuffers() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vboVertices) glDeleteBuffers(1, &vboVertices);
    if (vboIndices) glDeleteBuffers(1, &vboIndices);
    if (vboNormals) glDeleteBuffers(1, &vboNormals);
    vao = vboVertices = vboIndices = vboNormals = 0;
}