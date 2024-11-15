#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include <mesh.hpp>
#include <objloader.hpp>

Mesh::Mesh(vector<glm::vec3> indexed_vertices_in, vector<unsigned short> indices_in, int id_in = 0){
    indexed_vertices = indexed_vertices_in;
    indices = indices_in;
    id = id_in;
}

void Mesh::init(){
    glGenVertexArrays(1, &vaoBuffer);
    glGenBuffers(1, &vertexbuffer);
    glGenBuffers(1, &elementbuffer);

    glBindVertexArray(vaoBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(
        id,                  // attribute
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::draw(){
    glBindVertexArray(vaoBuffer);
    glEnableVertexAttribArray(id);

    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,      // mode
        indices.size(),    // count
        GL_UNSIGNED_SHORT,   // type
        (void*)0           // element array buffer offset
    );

    glDisableVertexAttribArray(id);
    glBindVertexArray(0);
}

void Mesh::deleteBuffer(){
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteVertexArrays(1, &vaoBuffer);
}