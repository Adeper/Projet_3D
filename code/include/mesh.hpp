#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <string>
#include <vector>

class Mesh{
    private :
        GLuint vertexbuffer;
        GLuint elementbuffer;
        GLuint vaoBuffer;
        std::vector<unsigned short> indices;
        std::vector<glm::vec3> indexed_vertices;
        int id;

    public :
        Mesh(std::vector<glm::vec3>, std::vector<unsigned short>, int);
        void init();
        void draw(GLuint programID);
        void deleteBuffer();
};

#endif