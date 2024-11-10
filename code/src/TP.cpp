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

bool globalInit();
GLFWwindow* initWindow();
void initImgui();

GLuint computeNoiseProgram, vertexNoiseProgram, fragmentNoiseProgram;
GLuint noiseTexture; 
GLuint noiseFramebuffer, quadVAO, quadVBO;
GLuint screenShader;

bool useComputeShader;

// Paramètres du bruit 
int noiseType = 0; // 0 = Perlin, 1 = Simplex, 2 = Coherent, 3 = Diamond Square
float scale = 1.0f; // Echelle du bruit
float gain = 0.5f; // Gain du bruit
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

    // Init ImGUI
    initImgui();

    // Charger les shaders
    if (glewIsSupported("GL_VERSION_4_3")) {
        computeNoiseProgram = loadComputeShader("noise_compute.glsl");
        useComputeShader = true;
        std::cout << "Utilisation du compute shader pour le bruit" << std::endl;
        
        // Création de la texture de bruit
        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 200, 200, 0, GL_RGBA, GL_FLOAT, nullptr); 

    } else {
        vertexNoiseProgram = loadVertexShader("noise_vertex.glsl");
        fragmentNoiseProgram = loadFragmentShader("noise_fragment.glsl");

        useComputeShader = false;
        std::cout << "Utilisation du fragment shader pour le bruit" << std::endl;

        // Créez un framebuffer pour dessiner la texture de bruit
        GLuint noiseFramebuffer;
        glGenFramebuffers(1, &noiseFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);

        // Créez une texture cible pour le framebuffer
        glGenTextures(1, &noiseTexture);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 200, 200, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, noiseTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Erreur: Framebuffer incomplet !" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Débind le framebuffer pour l'instant

        // Configurez les données de sommets pour un quad couvrant tout l'écran
        float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
 
    }
    // Charger le shader d'affichage pour afficher la texture de bruit à l'écran
    screenShader = LoadShaders("screen_vertex.glsl", "screen_fragment.glsl");
    
    do {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Imgui 
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Fenêtre de réglage du bruit
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Paramètres du bruit");

        const char* noiseTypes[] = { "Perlin", "Simplex", "Coherent", "Diamond Square" };
        ImGui::Combo("Type de bruit", &noiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes));
        ImGui::SliderFloat("Echelle", &scale, 0.0f, 10.0f);
        ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
        ImGui::SliderInt("Octaves", &octaves, 1, 10);
        ImGui::SliderFloat("Persistance", &persistence, 0.0f, 10.0f);
        ImGui::SliderFloat("Puissance", &power, 0.0f, 10.0f);

        if (ImGui::Button("Générer"))
        {
            seed = rand();

            if (useComputeShader){
                glUseProgram(computeNoiseProgram);
                
                // Envoie des paramètres du bruit au compute shader
                glUniform1i(glGetUniformLocation(computeNoiseProgram, "noiseType"), noiseType);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noiseScale"), scale);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noiseGain"), gain);
                glUniform1i(glGetUniformLocation(computeNoiseProgram, "noiseOctaves"), octaves);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noisePersistence"), persistence);
                glUniform1f(glGetUniformLocation(computeNoiseProgram, "noisePower"), power);
                glUniform1i(glGetUniformLocation(computeNoiseProgram, "seed"), seed);

                // Bind la texture de bruit
                glBindImageTexture(0, noiseTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
                glDispatchCompute(25, 25, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            else
            {
                // Utilisation du fragment shader pour générer la texture de bruit
                glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);
                glViewport(0, 0, 200, 200); // Ajustez la vue au cadre de la texture

                glUseProgram(fragmentNoiseProgram);
                glUniform1i(glGetUniformLocation(fragmentNoiseProgram, "noiseType"), noiseType);
                glUniform1f(glGetUniformLocation(fragmentNoiseProgram, "noiseScale"), scale);
                glUniform1f(glGetUniformLocation(fragmentNoiseProgram, "noiseGain"), gain);
                glUniform1i(glGetUniformLocation(fragmentNoiseProgram, "noiseOctaves"), octaves);
                glUniform1f(glGetUniformLocation(fragmentNoiseProgram, "noisePersistence"), persistence);
                glUniform1f(glGetUniformLocation(fragmentNoiseProgram, "noisePower"), power);
                glUniform1i(glGetUniformLocation(fragmentNoiseProgram, "seed"), seed);

                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindFramebuffer(GL_FRAMEBUFFER, 0); // Retour à l'écran principal
                glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Réinitialisez la vue
            }

            regenerate = false;
        }

        ImGui::SameLine();
        ImGui::Text("Seed : %d", seed);

        ImGui::End();

        // Bind the default framebuffer to display to the screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Revenir au framebuffer par défaut (écran)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Nettoyer l'écran

        // Utiliser le shader d'affichage (celui qui affiche la texture générée)
        glUseProgram(screenShader); 
        glUniform1i(glGetUniformLocation(screenShader, "noiseTexture"), 0); // Associer l'uniforme "noiseTexture" à l'unité de texture 0

        // Activer l'unité de texture 0 et lier la texture de bruit générée
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noiseTexture); 

        // Dessiner le quad plein écran avec la texture de bruit
        glBindVertexArray(quadVAO); 
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Fenêtre de visualisation du bruit
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Visualisation du bruit");

        // Affichage de l'aperçu du bruit (Placeholder pour le moment)
        ImGui::Text("Aperçu du bruit généré ici...");
        ImGui::Image((void*)(intptr_t)noiseTexture, ImVec2(200, 200));


        if (ImGui::Button("Importer"))
        {
            // Importer le bruit
        }
        ImGui::SameLine();
        if (ImGui::Button("Exporter"))
        {
            // Exporter le bruit
        }

        ImGui::End();

        // Renders the ImGUI elements
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    // Deletes all ImGUI instances
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (useComputeShader){
        glDeleteProgram(computeNoiseProgram);
    }
    else{
        glDeleteProgram(vertexNoiseProgram);
        glDeleteProgram(fragmentNoiseProgram);
    }
    glDeleteProgram(screenShader); // Ajout pour supprimer screenShader

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

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


