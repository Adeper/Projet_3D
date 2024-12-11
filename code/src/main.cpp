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
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

double lastTime = glfwGetTime(); // Temps écoulé depuis le début
int nbFrames = 0;               // Compteur de frames
int fps = 0;                    // FPS calculés pour affichage

// Bool pour afficher les fenêtres ImGui
bool show_tuto_window = true;
bool show_noise_window = false;
bool show_noise_visible_window = false;
bool show_camera_window = false;
bool show_plane_window = false;

// Touches release/pressed:
bool isNpressed = false;
bool isVpressed = false;
bool isCpressed = false;
bool isPpressed = false;
bool isTpressed = false;

GLuint computeNoiseProgram, vertexNoiseProgram, fragmentNoiseProgram;
GLuint noiseTexture; 
GLuint noiseFramebuffer, noiseFragmentProgram, quadVAO, quadVBO;

int width = 512;
int height = 512;

bool globalInit();
GLFWwindow* initWindow();
void initImgui();
void tutoImgui();
void processInput();

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

        if (show_tuto_window)
            tutoImgui(); // Affiche la fenêtre d'explication

        if (show_noise_window)
            noise.parametersInterface(); // Interface ImGui pour les paramètres du bruit
        
        noise.setUpParameters(); // Met à jour les paramètres du bruit

        if (show_noise_visible_window)
            noise.noiseInterface(); // Affiche la texture dans l'interface ImGui
    
        if (show_plane_window)
            terrain.showImGuiInterface(); // Interface ImGui pour les paramètres du plan

        if (show_camera_window)
            mainCamera.updateInterface(deltaTime); // Interface ImGui pour les paramètres de la caméra

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

        processInput();

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
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
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

void processInput(){
    
    // Si on appuie sur la touche N on ouvre les paramètres du bruit
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && !isNpressed) {
        show_noise_window = !show_noise_window;
        isNpressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_RELEASE) {
        isNpressed = false;
    }

    // Si on appuie sur la touche V on ouvre l'aperçu du bruit
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !isVpressed) {
        show_noise_visible_window = !show_noise_visible_window;
        isVpressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        isVpressed = false;
    }

    // Si on appuie sur la touche P on ouvre les paramètres du plan
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !isPpressed) {
        show_plane_window = !show_plane_window;
        isPpressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        isPpressed = false;
    }

    // Si on appuie sur la touche C on ouvre les paramètres de la caméra
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !isCpressed) {
        show_camera_window = !show_camera_window;
        isCpressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        isCpressed = false;
    }

    // Si on appuie sur la touche T on ouvre le guide d'utilisation
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !isTpressed) {
        show_tuto_window = !show_tuto_window;
        isTpressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        isTpressed = false;
    }
}

// Fenêtre imgui pour expliquer notre projet
void tutoImgui() {

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always); // Taille fixe
    ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH / 2, SCR_HEIGHT / 2), ImGuiCond_Always); // Centrée

    ImGui::Begin("Guide d'utilisation");

    ImGui::TextWrapped("Bienvenue dans le projet de génération procédurale de terrain et de routes ! Voici un guide rapide pour naviguer dans les fonctionnalités :");

    ImGui::Separator();

    ImGui::TextWrapped("1. Fenêtre des paramètres pour le bruit (Touche N) :");
    ImGui::BulletText("Modifiez les paramètres de bruit comme le type de bruit, le gain, les octaves, etc.");
    ImGui::BulletText("Permet de visualiser et ajuster la génération procédurale du terrain.");

    ImGui::TextWrapped("2. Fenêtre d'aperçu du bruit (Touche V) :");
    ImGui::BulletText("Affiche un aperçu visuel de la heightmap générée par le bruit.");
    ImGui::BulletText("Permet de vérifier les résultats en temps réel.");

    ImGui::TextWrapped("3. Fenêtre des paramètres pour le plan (Touche P) :");
    ImGui::BulletText("Réglez la taille, la résolution, et d'autres propriétés du plan.");
    ImGui::BulletText("Permet aussi de modifier les limites de textures (herbe, roche, etc.).");

    ImGui::TextWrapped("4. Fenêtre des paramètres de la caméra (Touche C) :");
    ImGui::BulletText("Ajustez la position, l'angle et les paramètres de la caméra.");
    ImGui::BulletText("Permet également d'inverser les axes ou changer de mode de caméra.");

    ImGui::Separator();
    ImGui::TextWrapped("Contrôles généraux :");
    ImGui::BulletText("Appuyez sur T pour rouvrir ce guide.");
    ImGui::BulletText("Échappe pour quitter l'application.");

    ImGui::Separator();

    if (ImGui::Button("OK")) {
        show_tuto_window = false;
    }

    ImGui::End();
}