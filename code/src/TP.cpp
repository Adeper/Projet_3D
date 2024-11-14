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
#include <Actor.hpp>
#include <Camera.hpp>
#include <Skybox.hpp>

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
void setUPVAOVBO();

void updateLightPosition(GLuint _lightID);

bool useComputeShader;

// Paramètres du bruit 
int noiseType = 0; // 0 = Perlin, 1 = Simplex, 2 = Coherent, 3 = Diamond Square
float scale = 1.0f; // Echelle du bruit
float gain = 1.0f; // Gain du bruit
int octaves = 4; // Nombre d'octaves
float persistence = 2.0f; // Persistance du bruit
float power = 1.0f; // Puissance du bruit
int seed = 0; // Graine du bruit
bool regenerate = false; // Regénérer le bruit


int main(void)
{
    if (!globalInit())
    {
        return -1;
    }

    initImgui();

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    GLuint programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    GLuint colorID = glGetUniformLocation(programID, "color_Mesh");

    if (glewIsSupported("GL_VERSION_4_3")) {
        computeNoiseProgram = loadComputeShader("noise_compute.glsl");
        useComputeShader = true;
        std::cout << "Utilisation du compute shader pour le bruit" << std::endl;

        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindImageTexture(0, noiseTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    } else {
        noiseFragmentProgram = LoadShaders("noise_vertex.glsl", "noise_fragment.glsl");
        useComputeShader = false;
        std::cout << "Utilisation du fragment shader pour le bruit" << std::endl;
        setUPVAOVBO();

        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 200, 200, 0, GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &noiseFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, noiseTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Erreur : Framebuffer incomplet !" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ObjController map;
    Actor target;

    map.loadObj("../data/myMap2.obj", glm::vec3(0.6f, 0.5f, 0.3f), colorID);
    target.load("../data/cameraTarget.obj", glm::vec3(0.8f, 0.5f, 0.4f), colorID);

    glUseProgram(programID);

    Camera mainCamera;
    mainCamera.init();

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    //VSync - avoid having 3000 fps
    glfwSwapInterval(1);

    // ==== Skybox ====
    std::vector<std::string> faces = {
        "../data/skybox/right.jpg",
        "../data/skybox/left.jpg",
        "../data/skybox/top.jpg",
        "../data/skybox/bottom.jpg",
        "../data/skybox/front.jpg",
        "../data/skybox/back.jpg"
    };
    Skybox skybox(faces);

    do {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Nettoyage avant de dessiner
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Paramètres du bruit");

        const char* noiseTypes[] = { "Bruit par défaut", "Perlin", "Simplex", "Coherent", "Diamond Square" };
        
        static int previousNoiseType = noiseType;
        static float previousScale = scale;
        static float previousGain = gain;
        static int previousOctaves = octaves;
        static float previousPersistence = persistence;
        static float previousPower = power;

        // Les sliders et le combo pour ajuster les paramètres
        ImGui::Combo("Type de bruit", &noiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes));
        ImGui::SliderFloat("Echelle", &scale, 1.0f, 100.0f);
        ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
        ImGui::SliderInt("Octaves", &octaves, 1, 10);
        ImGui::SliderFloat("Persistance", &persistence, 0.3f, 0.7f);
        ImGui::SliderFloat("Puissance", &power, 1.0f, 10.0f);

        // Détection des changements de paramètres
        bool hasChanged = (noiseType != previousNoiseType ||
                        scale != previousScale ||
                        gain != previousGain ||
                        octaves != previousOctaves ||
                        persistence != previousPersistence ||
                        power != previousPower);

        if (hasChanged || regenerate) {
            // Mettre à jour la texture en fonction des paramètres actuels
            if (useComputeShader) {
                glUseProgram(computeNoiseProgram);
                glUniform1i(glGetUniformLocation(computeNoiseProgram, "noiseType"), noiseType);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noiseScale"), scale);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noiseGain"), gain);
                glUniform1i(glGetUniformLocation(computeNoiseProgram, "noiseOctaves"), octaves);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noisePersistence"), persistence);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noisePower"), power);
                glUniform1i(glGetUniformLocation(computeNoiseProgram, "seed"), seed);

                glDispatchCompute(width / 16, height / 16, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            } else {
                glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);
                glUseProgram(noiseFragmentProgram);

                glUniform1i(glGetUniformLocation(noiseFragmentProgram, "noiseType"), noiseType);
                glUniform1f(glGetUniformLocation(noiseFragmentProgram, "noiseScale"), scale);
                glUniform1f(glGetUniformLocation(noiseFragmentProgram, "noiseGain"), gain);
                glUniform1i(glGetUniformLocation(noiseFragmentProgram, "noiseOctaves"), octaves);
                glUniform1f(glGetUniformLocation(noiseFragmentProgram, "noisePersistence"), persistence);
                glUniform1f(glGetUniformLocation(noiseFragmentProgram, "noisePower"), power);
                glUniform1i(glGetUniformLocation(noiseFragmentProgram, "seed"), seed);

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            // Mise à jour de l'état des sliders
            previousNoiseType = noiseType;
            previousScale = scale;
            previousGain = gain;
            previousOctaves = octaves;
            previousPersistence = persistence;
            previousPower = power;

            regenerate = false;
        }
        if (ImGui::Button("Regénérer")) {
            regenerate = true;
            seed = rand();
        }
        ImGui::SameLine();
        ImGui::Text("Seed : %d", seed);
        ImGui::End();

        // Fenêtre pour l'aperçu du bruit
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Visualisation du bruit");
        ImGui::Text("Aperçu du bruit généré ici...");
        ImGui::Image((void*)(intptr_t)noiseTexture, ImVec2(200, 200));

        // Boutons pour importer/exporter
        if (ImGui::Button("Importer")) {
            // Code d'importation
        }
        ImGui::SameLine();
        if (ImGui::Button("Exporter")) {
            // Code d'exportation
        }

        ImGui::End();

        target.update(deltaTime, window, mainCamera.getRotation());
        mainCamera.update(deltaTime, window);


        glm::mat4 viewMatrix = mainCamera.getViewMatrix();
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &viewMatrix[0][0]);

        //View
        updateLightPosition(LightID);

        //map.updateViewAndDraw(mainCamera, MatrixID, ModelMatrixID);
        target.updateViewAndDraw(mainCamera, MatrixID, ModelMatrixID);

        skybox.draw(mainCamera.getViewMatrix(), mainCamera.getProjectionMatrix());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    if (useComputeShader) {
        glDeleteProgram(computeNoiseProgram);
    }
    else {
        glDeleteProgram(noiseFragmentProgram);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteFramebuffers(1, &noiseFramebuffer);
    }

    glDeleteProgram(programID);

    map.deleteBuffer();
    target.destroy();

    glDeleteTextures(1, &noiseTexture);

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

void setUPVAOVBO(){
    float quadVertices[] = {
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Configurer les attributs de position et de coordonnées de texture
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // Texture coords
    glBindVertexArray(0);
}

void updateLightPosition(GLuint _lightID)
{
    const glm::vec3 lightPos = glm::vec3(4.f, 90.f, 4.f);
    glUniform3f(_lightID, lightPos.x, lightPos.y, lightPos.z);
}