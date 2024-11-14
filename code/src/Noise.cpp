#include "Noise.hpp"

// Constructeur
void Noise::init(){
    
    noiseType = 0; 
    scale = 1.0f;
    gain = 1.0f; 
    octaves = 4; 
    persistence = 2.0f; 
    power = 1.0f; 

    width = 1024;
    height = 1024;

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

int Noise::getNoiseType(){
    return noiseType;
}

void Noise::setNoiseType(int newNoiseType){
    noiseType = newNoiseType;
}

float Noise::getScale(){
    return scale;
}

void Noise::setScale(float newScale){
    scale = newScale;
}

float Noise::getGain(){
    return gain;
}

void Noise::setGain(float newGain){
    gain = newGain;
}

int Noise::getOctaves(){
    return octaves;
}

void Noise::setOctaves(int newOctaves){
    octaves = newOctaves;
}

float Noise::getPersistence(){
    return persistence;
}

void Noise::setPersistence(float newPersistence){
    persistence = newPersistence;
}

float Noise::getPower(){
    return power;
}

void Noise::setPower(float newPower){
    power = newPower;
}

int Noise::getWidth() {
    return width;
}

void Noise::setWidth(int newWidth) {
    width = newWidth;
    hasChangedRes = true;
}

int Noise::getHeight() {
    return height;
}

void Noise::setHeight(int newHeight) {
    height = newHeight;
    hasChangedRes = true;
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

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
    glUniform1f(glGetUniformLocation(programID, "noiseScale"), scale);
    glUniform1f(glGetUniformLocation(programID, "noiseGain"), gain);
    glUniform1i(glGetUniformLocation(programID, "noiseOctaves"), octaves);
    glUniform1f(glGetUniformLocation(programID, "noisePersistence"), persistence);
    glUniform1f(glGetUniformLocation(programID, "noisePower"), power);
    glUniform2i(glGetUniformLocation(programID, "resolution"), width, height);
}

void Noise::parametersInterface(){

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Paramètres du bruit");

    const char* noiseTypes[] = { "Perlin basique", "Perlin 2D", "Perlin 3D", "Perlin 4D" };
    
    static int previousNoiseType = noiseType;
    static float previousScale = scale;
    static float previousGain = gain;
    static int previousOctaves = octaves;
    static float previousPersistence = persistence;
    static float previousPower = power;
    static int previousWidth = width;
    static int previousHeight = height;

    // Les sliders et le combo pour ajuster les paramètres
    ImGui::Combo("Type de bruit", &noiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes));
    ImGui::SliderFloat("Echelle", &scale, 1.0f, 100.0f);
    ImGui::SliderFloat("Gain", &gain, 0.0f, 1.0f);
    ImGui::SliderInt("Octaves", &octaves, 1, 10);
    ImGui::SliderFloat("Persistance", &persistence, 0.3f, 2.0f);
    ImGui::SliderFloat("Puissance", &power, 1.0f, 10.0f);

    // Les sliders pour la résolution : vérifier que width et height sont des multiples de 16
    // à cause du compute shader

    int units_multiple = width / 16;  

    // Slider pour la résolution
    if (ImGui::SliderInt("Résolution", &units_multiple, 128 / 16, 2048 / 16)) {
        width = units_multiple * 16;
        height = units_multiple * 16; 
    }

    ImGui::Text("Résolution actuelle : %dx%d", width, height);

    // Détection des changements de résolution
    hasChangedRes = (width != previousWidth || height != previousHeight);

    if (hasChangedRes) {
        // Recréer la texture et mets à jour les variables
        initTexture();
        setBindingTexture();
        if (useComputeShader){
            glUseProgram(programID);
            sendParameters();
            glDispatchCompute(width / 16, height / 16, 1);
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
        previousWidth = width;
        previousHeight = height;
    }

    // Détection des changements
    hasChanged = (noiseType != previousNoiseType ||
                    scale != previousScale ||
                    gain != previousGain ||
                    octaves != previousOctaves ||
                    persistence != previousPersistence ||
                    power != previousPower);

    if (hasChanged) {
        // Mettre à jour la texture en fonction des paramètres actuels
        if (useComputeShader){
            glUseProgram(programID);

            sendParameters();

            glDispatchCompute(width / 16, height / 16, 1);
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
        previousScale = scale;
        previousGain = gain;
        previousOctaves = octaves;
        previousPersistence = persistence;
        previousPower = power;
    }

    ImGui::End();
}

void Noise::noiseInterface(){

    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Visualisation du bruit");

    ImGui::Text("Aperçu du bruit généré ici...");
    ImGui::Image((void*)(intptr_t)noiseTexture, ImVec2(450, 450));

    // Boutons pour importer/exporter
    if (ImGui::Button("Importer")) {
        // Code d'importation
    }
    ImGui::SameLine();
    if (ImGui::Button("Exporter")) {
        // Code d'exportation
    }

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

void Noise::loadTexture(const char* path) {
    // Charger l'image 
}

void Noise::saveTexture(const char* path) {
    // Sauvegarde de l'image'
}