#include "Plane.hpp"
#include <iostream>
#include <glm/glm.hpp>

Plane::Plane(float width, float depth, int resolution, const glm::vec3& _color, GLuint _colorID)
    : width(width), depth(depth), resolution(resolution), color(_color), colorID(_colorID) {
    generatePlane();
}

Plane::~Plane() {
    glDeleteBuffers(1, &m_vertexbufferID);
    glDeleteBuffers(1, &m_uvbufferID);
    glDeleteBuffers(1, &m_normalbufferID);
}

void Plane::generatePlane() {
    int vertexCount = (resolution + 1) * (resolution + 1);
    int indexCount = resolution * resolution * 6;

    vertices.resize(vertexCount * 3); 
    normals.resize(vertexCount * 3, 0.0f); 
    indices.resize(indexCount);
    uvs.resize(vertexCount * 2);

    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;
    float dx = width / resolution;
    float dz = depth / resolution;

    int vertexIndex = 0;
    int uvIndex = 0;
    for (int z = 0; z <= resolution; ++z) {
        for (int x = 0; x <= resolution; ++x) {
            float xPos = -halfWidth + x * dx;
            float zPos = -halfDepth + z * dz;

            // Position du sommet
            vertices[vertexIndex++] = xPos;
            vertices[vertexIndex++] = 0.0f;
            vertices[vertexIndex++] = zPos;

            // Coordonnées UV
            uvs[uvIndex++] = static_cast<float>(x) / resolution; // u
            uvs[uvIndex++] = static_cast<float>(z) / resolution; // v
        }
    }

    int index = 0;
    for (int z = 0; z < resolution; ++z) {
        for (int x = 0; x < resolution; ++x) {
            int start = z * (resolution + 1) + x;
            indices[index++] = start;
            indices[index++] = start + resolution + 1;
            indices[index++] = start + 1;

            indices[index++] = start + 1;
            indices[index++] = start + resolution + 1;
            indices[index++] = start + resolution + 2;
        }
    }

    std::fill(normals.begin(), normals.end(), 0.0f);
    for (size_t i = 1; i < normals.size(); i += 3) {
        normals[i] = 1.0f;
    }

    glGenBuffers(1, &m_vertexbufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_uvbufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvbufferID);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float), uvs.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_normalbufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalbufferID);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
}


void Plane::draw(const Camera& _camera, GLuint _matrixID, GLuint _modelMatrixID) {
    updateView(_camera, _matrixID, _modelMatrixID);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID);
    glVertexAttribPointer(
        0,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, m_uvbufferID);
    glVertexAttribPointer(
        1,                                // attribute
        2,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, m_normalbufferID);
    glVertexAttribPointer(
        2,                                // attribute
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
}

void Plane::updateView(const Camera& _camera, GLuint _matrixID, GLuint _modelMatrixID){
    glm::mat4 projectionMatrix = _camera.getProjectionMatrix();
    glm::mat4 viewMatrix = _camera.getViewMatrix();
    glm::mat4 MVP = projectionMatrix * viewMatrix * m_modelMatrix;

    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glUniform3f(colorID, color.x, color.y, color.z);
    glUniformMatrix4fv(_matrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(_modelMatrixID, 1, GL_FALSE, &m_modelMatrix[0][0]);
}
