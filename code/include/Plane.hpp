#ifndef PLANE_HPP
#define PLANE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <Camera.hpp>
#include <shader.hpp>

class Plane {
public:
    Plane(float width, float depth, int resolution, const glm::vec3& _color, GLuint _colorID);
    ~Plane();

    void draw(const Camera& _camera, GLuint _matrixID, GLuint _modelMatrixID);

private:
    void generatePlane();
    void updateView(const Camera& _camera, GLuint _matrixID, GLuint _modelMatrixID);

    float width, depth;
    int resolution;

    GLuint colorID;
    GLuint m_vertexbufferID;
	GLuint m_uvbufferID;
	GLuint m_normalbufferID;

    glm::vec3 color;
    std::vector<float> vertices;   
    std::vector<unsigned int> indices;
    std::vector<float> normals;
    std::vector<float> uvs;

    glm::mat4 m_modelMatrix;
};

#endif
