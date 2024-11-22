#pragma once //askip c'est bien 

#ifndef NOISE_HPP
#define NOISE_HPP

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

// Include GLM
#include <glm/glm.hpp>

// Include ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Include shaders
#include <shader.hpp>

/* Classe pour générer le bruit sur une fenêtre ImGui en fonction de paramètres */

class Noise {

    private:

        int noiseType; // Type de bruit
        float gain; // Gain du bruit
        int octaves; // Nombre d'octaves
        float persistence; // Persistance du bruit
        float power; // UNLIMITED POWER
        
        int resolution;

        
        GLuint programID; 
        GLuint noiseTexture; 

        // Spécifique au fragment shader
        GLuint VAO, VBO;
        GLuint noiseFramebuffer;

        bool hasChanged; // Pour savoir si les paramètres ont changé
        bool hasChangedRes; // Pour savoir si la résolution a changé

    public:

        bool useComputeShader; // Pour savoir si on utilise le compute shader ou vertex/fragment shader

        // Setters et getters des paramètres du bruit

        int getNoiseType();
        void setNoiseType(int newNoiseType);

        float getGain();
        void setGain(float newGain);

        int getOctaves();
        void setOctaves(int newOctaves);

        float getPersistence();
        void setPersistence(float newPersistence);

        float getPower();
        void setPower(float newPower);

        int getResolution();
        void setResolution(int newResolution);

        // Quelques Set Up 

        void setProgramID(); // chargement des ou du shader
        void initTexture(); // initialisation de la texture où sera stocker le bruit
        void setBindingTexture(); // binding de la texture
        void initVAOVBO(); // initialisation des VAO et VBO
        void sendParameters(); // envoie des paramètres au shader

        // Constructeur

        void init();

        // Interface

        void parametersInterface();
        void noiseInterface();

        // Importation et exportation des textures

        std::string getJsonPath(const std::string& imagePath);
        
        void saveMetadata(const char* path);
        void loadMetadata(const char* path);
        void loadTexture(const char* path);
        void saveTexture(const char* path);

        // Destructeur
        void destroy();

};

#endif