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

// Paramètres de la caméra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

GLuint computeNoiseProgram, vertexNoiseProgram, fragmentNoiseProgram;
GLuint noiseTexture; 
GLuint noiseFramebuffer, noiseFragmentProgram, quadVAO, quadVBO;

int width = 512;
int height = 512;

bool globalInit();
GLFWwindow* initWindow();
void initImgui();
void setUPVAOVBO();

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

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major > 4 || (major == 4 && minor >= 3))  {
        computeNoiseProgram = loadComputeShader("noise_compute.glsl");
        useComputeShader = true;
        std::cout << "OpenGL est au moins en version 4.3" << std::endl;
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
        std::cerr << "OpenGL est inférieur à la version 4.3" << std::endl;
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

    do {
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
        ImGui::SliderFloat("Persistance", &persistence, 0.3f, 2.0f);
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
        ImGui::Image((void*)(intptr_t)noiseTexture, ImVec2(400, 400));

        // Boutons pour importer/exporter
        if (ImGui::Button("Importer")) {
            // Code d'importation
        }
        ImGui::SameLine();
        if (ImGui::Button("Exporter")) {
            // Code d'exportation
        }

        ImGui::End();


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

    if (useComputeShader) {
        glDeleteProgram(computeNoiseProgram);
    }
    else {
        glDeleteProgram(noiseFragmentProgram);
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteFramebuffers(1, &noiseFramebuffer);
    }

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // à changer si OpenGL inférieur à 4.3
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


