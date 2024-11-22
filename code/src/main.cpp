// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>

// Include ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Include shaders
#include <shader.hpp>
#include <Camera.hpp>
#include <Skybox.hpp>
#include <Plane.hpp>
#include <Noise.hpp>

// Paramètres de la caméra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

GLuint computeNoiseProgram, vertexNoiseProgram, fragmentNoiseProgram;
GLuint noiseTexture; 
GLuint noiseFramebuffer, noiseFragmentProgram, quadVAO, quadVBO;

int width = 512;
int height = 512;

bool globalInit();
GLFWwindow* initWindow();
void initImgui();

void updateLightPosition(GLuint _lightPosID, GLuint _lightColorID);


int main(void)
{
    if (!globalInit())
    {
        return -1;
    }

    initImgui();

    // Création du programme de calcul
    Noise noise;
    noise.init(); // Initialisation des paramètres

    noise.setProgramID(); // Charge les shaders appropriés
    noise.initTexture(); // Initialise la texture
    noise.setBindingTexture(); // Prépare le binding de la texture

    if (!noise.useComputeShader) {
        noise.initVAOVBO(); // Initialise le VAO et le VBO
    }

    GLuint programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    GLuint LightPosID = glGetUniformLocation(programID, "LightPosition_worldspace");
    GLuint LightColorID = glGetUniformLocation(programID, "LightColor_worldspace");

    Camera mainCamera;
    mainCamera.init();

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    //VSync - avoid having 3000 fps
    glfwSwapInterval(1);

    // ==== Skybox ====
    std::vector<std::string> faces = {
        "../data/skybox/standard_tuto/px.jpg",
        "../data/skybox/standard_tuto/nx.jpg",
        "../data/skybox/standard_tuto/py.jpg",
        "../data/skybox/standard_tuto/ny.jpg",
        "../data/skybox/standard_tuto/pz.jpg",
        "../data/skybox/standard_tuto/nz.jpg"
    };
    Skybox skybox(faces);

    Plane terrain(10.f, 10, &mainCamera);

    glDisable(GL_CULL_FACE);

    do {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Nettoyage avant de dessiner
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        noise.parametersInterface(); // Interface ImGui pour les paramètres du bruit
        noise.noiseInterface(); // Affiche la texture dans l'interface ImGui

        mainCamera.update(deltaTime, window);

        //View
        updateLightPosition(LightPosID, LightColorID);

        skybox.draw(mainCamera.getViewMatrix(), mainCamera.getProjectionMatrix());

        terrain.update();
        
        noise.setResolution(terrain.getResolution());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    noise.destroy(); // Libère les ressources

    return 0;
}

bool globalInit()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return false;
    }

    window = initWindow();
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return false;
    }

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    return true;
}

GLFWwindow* initWindow()
{
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // Open a window and create its OpenGL context
    GLFWwindow* createdWindow = glfwCreateWindow(1500, 1000, "Projet 3D", NULL, NULL);
    if (createdWindow != NULL) {
        glfwMakeContextCurrent(createdWindow);
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    return createdWindow;
}


void initImgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void updateLightPosition(GLuint _lightPosID, GLuint _lightColorID)
{
    const glm::vec3 lightPos = glm::vec3(0.f, 40.f, 0.f);
    glUniform3f(_lightPosID, lightPos.x, lightPos.y, lightPos.z);

    const glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);
    glUniform3f(_lightColorID, lightColor.x, lightColor.y, lightColor.z);
}