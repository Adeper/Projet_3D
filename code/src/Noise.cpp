#include "Noise.hpp"
#include <stb_image.h>

#ifdef __clang__
#define STBIWDEF static inline
#endif
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Inclue tinyfiledialogs
#include <tinyfiledialogs.h>

// Constructeur
void Noise::init(){
    
    noiseType = 0; 
    gain = 1.0f; 
    octaves = 4; 
    persistence = 2.0f; 
    power = 1.0f; 

    resolution = 10;

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major > 4 || (major == 4 && minor >= 3)){
        useComputeShader = true;
    }
    else {
        useComputeShader = false;
    }

}

// Setters et getters des paramètres du bruit

int Noise::getNoiseType() const{
    return noiseType;
}

void Noise::setNoiseType(int newNoiseType){
    noiseType = newNoiseType;
}

float Noise::getGain() const{
    return gain;
}

void Noise::setGain(float newGain){
    gain = newGain;
}

int Noise::getOctaves() const{
    return octaves;
}

void Noise::setOctaves(int newOctaves){
    octaves = newOctaves;
}

float Noise::getPersistence() const{
    return persistence;
}

void Noise::setPersistence(float newPersistence){
    persistence = newPersistence;
}

float Noise::getPower() const{
    return power;
}

void Noise::setPower(float newPower){
    power = newPower;
}

int Noise::getResolution() const{
    return resolution;
}

void Noise::setResolution(int newResolution) {
    resolution = newResolution;
    hasChangedRes = true;
}

GLuint Noise::getTextureNoise() const{
    return noiseTexture;
}

// Quelques Set Up

void Noise::setProgramID(){
    if (useComputeShader){
        programID = loadComputeShader("noise_compute.glsl");
    }
    else {
        programID = LoadShaders("noise_vertex.glsl", "noise_fragment.glsl");
    }
}

void Noise::initTexture(){

    if (glIsTexture(noiseTexture)) {
        glDeleteTextures(1, &noiseTexture);

        if(!useComputeShader){
            glDeleteFramebuffers(1, &noiseFramebuffer);
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);   
        }
    }

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Noise::setBindingTexture(){
    if (useComputeShader){
        glBindImageTexture(0, noiseTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    }
    else {
        glGenFramebuffers(1, &noiseFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, noiseTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Erreur : Framebuffer incomplet !" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }
}

void Noise::initVAOVBO(){

    if (glIsVertexArray(VAO)) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    float quadVertices[] = {
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Configurer les attributs de position et de coordonnées de texture
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // Texture coords
    glBindVertexArray(0);
}

// Envoie des paramètres au shader
void Noise::sendParameters(){
    glUseProgram(programID);
    glUniform1i(glGetUniformLocation(programID, "noiseType"), noiseType);
    glUniform1f(glGetUniformLocation(programID, "noiseGain"), gain);
    glUniform1i(glGetUniformLocation(programID, "noiseOctaves"), octaves);
    glUniform1f(glGetUniformLocation(programID, "noisePersistence"), persistence);
    glUniform1f(glGetUniformLocation(programID, "noisePower"), power);
    glUniform1i(glGetUniformLocation(programID, "resolution"), resolution);
}

void Noise::parametersInterface(){

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Paramètres du bruit");

    const char* noiseTypes[] = { "Perlin basique", "Perlin 2D", "Perlin 3D", "Perlin 4D" };

    // Les sliders et le combo pour ajuster les paramètres
    ImGui::Combo("Type de bruit", &noiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes));
    ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
    ImGui::SliderInt("Octaves", &octaves, 1, 10);
    ImGui::SliderFloat("Persistance", &persistence, 0.3f, 2.0f);
    ImGui::SliderFloat("Puissance", &power, 1.0f, 10.0f);

    ImGui::End();
}

void Noise::setUpParameters(){

    static int previousNoiseType = noiseType;
    static float previousGain = gain;
    static int previousOctaves = octaves;
    static float previousPersistence = persistence;
    static float previousPower = power;

    if (hasChangedRes) {
        // Recréer la texture et mets à jour les variables
        initTexture();
        setBindingTexture();
        if (useComputeShader){
            glUseProgram(programID);
            sendParameters();
            glDispatchCompute(resolution / 16, resolution / 16, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            
        }
        else {
            initVAOVBO();
            glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);
            glUseProgram(programID);
            sendParameters();
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    // Détection des changements
    hasChanged = (noiseType != previousNoiseType ||
                    gain != previousGain ||
                    octaves != previousOctaves ||
                    persistence != previousPersistence ||
                    power != previousPower);

    if (hasChanged) {
        // Mettre à jour la texture en fonction des paramètres actuels
        if (useComputeShader){
            glUseProgram(programID);

            sendParameters();

            glDispatchCompute(resolution / 16, resolution / 16, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, noiseFramebuffer);
            glUseProgram(programID);

            sendParameters();

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Mise à jour de l'état des sliders
        previousNoiseType = noiseType;
        previousGain = gain;
        previousOctaves = octaves;
        previousPersistence = persistence;
        previousPower = power;
    }
}

void Noise::noiseInterface(){

    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Visualisation du bruit");

    ImGui::Text("Aperçu du bruit généré ici...");
    ImGui::Image((void*)(intptr_t)noiseTexture, ImVec2(450, 450));

    ImGui::End();
}

void Noise::destroy(){
    if (useComputeShader){
        glDeleteProgram(programID);
    }
    else {
        glDeleteProgram(programID);
        glDeleteFramebuffers(1, &noiseFramebuffer);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);   
    }

    glDeleteTextures(1, &noiseTexture);
}