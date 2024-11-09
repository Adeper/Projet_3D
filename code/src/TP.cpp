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

// Paramètres de la caméra
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool globalInit();
GLFWwindow* initWindow();
void initImgui();
GLuint loadComputeShader(const char* shaderPath);
GLuint createComputeProgram();
GLuint computeProgram, noiseTexture;

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

    // Charger et compiler le compute shader
    computeProgram = createComputeProgram();

    // Création de la texture de bruit
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 100, 100, 0, GL_RGBA, GL_FLOAT, nullptr); 
    
    do {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Imgui 
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Fenêtre de réglage du bruit
        ImGui::SetNextWindowSize(ImVec2(1000, 800), ImGuiCond_FirstUseEver);
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
            glUseProgram(computeProgram);

            // Envoi des paramètres du bruit au compute shader
            glUniform1i(glGetUniformLocation(computeProgram, "noiseType"), noiseType);
            glUniform1f(glGetUniformLocation(computeProgram, "noiseScale"), scale);
            glUniform1f(glGetUniformLocation(computeProgram, "noiseGain"), gain);
            glUniform1i(glGetUniformLocation(computeProgram, "noiseOctaves"), octaves);
            glUniform1f(glGetUniformLocation(computeProgram, "noisePersistence"), persistence);
            glUniform1f(glGetUniformLocation(computeProgram, "noisePower"), power);
            glUniform1i(glGetUniformLocation(computeProgram, "seed"), seed);

            // Lancer le compute shader
            glActiveTexture(GL_TEXTURE0);
            glBindImageTexture(0, noiseTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(25, 25, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glDispatchCompute(25, 25, 1);  // 100 / 4 = 25 groupes de travail
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            regenerate = false;
        }

        ImGui::SameLine();
        ImGui::Text("Seed : %d", seed);

        ImGui::End();

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

GLuint loadComputeShader(const char* shaderPath){
    // 1. Charger le code GLSL du fichier
    std::ifstream shaderFile(shaderPath);
    if (!shaderFile.is_open()) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier " << shaderPath << std::endl;
        return 0;
    }
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    std::string shaderCode = shaderStream.str();
    const char* shaderSource = shaderCode.c_str();
    shaderFile.close();

    // 2. Créer et compiler le shader
    GLuint shaderID = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shaderID, 1, &shaderSource, nullptr);
    glCompileShader(shaderID);

    // 3. Vérifier les erreurs de compilation
    GLint success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(logLength);
        glGetShaderInfoLog(shaderID, logLength, nullptr, errorLog.data());
        std::cerr << "Erreur de compilation du compute shader : " << errorLog.data() << std::endl;
        std::cout << std::endl;
        glDeleteShader(shaderID);
        return 0;
    }

    return shaderID;

}

GLuint createComputeProgram() {
    // Charger et compiler le compute shader
    GLuint computeShader = loadComputeShader("noise.glsl");
    if (computeShader == 0) {
        return 0;
    }

    // Créer le programme et lier le shader
    GLuint computeProgram = glCreateProgram();
    glAttachShader(computeProgram, computeShader);
    glLinkProgram(computeProgram);

    // Vérifier les erreurs de linkage
    GLint success;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> errorLog(logLength);
        glGetProgramInfoLog(computeProgram, logLength, nullptr, errorLog.data());
        std::cerr << "Erreur de linkage du programme compute shader : " << errorLog.data() << std::endl;
        std::cout << std::endl;
        glDeleteProgram(computeProgram);
        glDeleteShader(computeShader);
        return 0;
    }

    // Le shader peut être détaché et supprimé après le linkage
    glDeleteShader(computeShader);

    return computeProgram;
}