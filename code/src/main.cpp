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
#include <BezierCurve.hpp>

// Paramètres de la caméra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

double lastTime = glfwGetTime(); // Temps écoulé depuis le début
int nbFrames = 0;               // Compteur de frames
int fps = 0;                    // FPS calculés pour affichage


GLuint computeNoiseProgram, vertexNoiseProgram, fragmentNoiseProgram;
GLuint noiseTexture; 
GLuint noiseFramebuffer, noiseFragmentProgram, quadVAO, quadVBO;

int width = 512;
int height = 512;

bool globalInit();
GLFWwindow* initWindow();
void initImgui();

int main(void)
{
    if (!globalInit())
    {
        return -1;
    }

    initImgui();

    GLuint programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    GLuint viewPosLoc = glGetUniformLocation(programID, "ViewPosition");

    // Création du programme de calcul
    Noise noise;
    noise.init(); // Initialisation des paramètres
    noise.useComputeShader = false;

    noise.setProgramID(); // Charge les shaders appropriés
    noise.initTexture(); // Initialise la texture
    noise.setBindingTexture(); // Prépare le binding de la texture

    if (!noise.useComputeShader) {
        noise.initVAOVBO(); // Initialise le VAO et le VBO
    }
    
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

    BezierCurve chemin(terrain.getResolution(), terrain.getSize());
    chemin.initControlPoints(glm::vec3(-2.5f, 0.0f, -2.5f), glm::vec3(2.5f, 0.0f, 2.5f), 3);

    glDisable(GL_CULL_FACE);

    do {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Calcul des FPS
        nbFrames++;
        if (currentFrame - lastTime >= 1.0) { // Compte les FPS chaque seconde
            fps = nbFrames;
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Nettoyage avant de dessiner
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparence de la fenêtre
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always); // Position : haut-gauche
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f); // Coins arrondis
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5)); // Espacement interne

        ImGui::Begin("Overlay", NULL, 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoFocusOnAppearing | 
            ImGuiWindowFlags_NoNav);

        // Choisir une couleur en fonction des FPS
        if (fps > 60)
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "FPS: %d", fps); // Vert pour > 60 FPS
        else if (fps > 30)
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "FPS: %d", fps); // Jaune pour 30-60 FPS
        else
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FPS: %d", fps); // Rouge pour < 30 FPS

        ImGui::End();
        ImGui::PopStyleVar(2); // Revenir aux styles par défaut

        noise.parametersInterface(); // Interface ImGui pour les paramètres du bruit
        noise.noiseInterface(); // Affiche la texture dans l'interface ImGui

        mainCamera.update(deltaTime, window);

        glm::vec3 cameraPos = mainCamera.getPosition();

        //View
        glUniform3fv(viewPosLoc, 1, &cameraPos[0]);

        glm::mat4 viewMatrix = mainCamera.getViewMatrix();
        glm::mat4 projMatrix = mainCamera.getProjectionMatrix();

        skybox.draw(viewMatrix, projMatrix);

        terrain.update();
        
        noise.setResolution(terrain.getResolution());
        terrain.setHeightMap(noise.getTextureNoise());
        
        chemin.update(terrain.getSize(), terrain.getResolution(), terrain.getHeightScale());
        
        chemin.draw(viewMatrix, projMatrix);


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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

        glfwSetFramebufferSizeCallback(createdWindow, framebuffer_size_callback);
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