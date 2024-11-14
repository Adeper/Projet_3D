#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Skybox {
    public:
        Skybox(const std::vector<std::string>& faces);
        ~Skybox();

        void draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    private:
        GLuint loadCubemap(const std::vector<std::string>& faces);
        GLuint createSkyboxVAO();

        GLuint m_cubemapTexture;
        GLuint m_skyboxVAO, m_skyboxVBO;
        GLuint m_shaderProgram;
};

#endif 