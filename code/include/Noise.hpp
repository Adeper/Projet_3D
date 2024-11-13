#pragma once //askip c'est bien 

#ifndef NOISE_HPP
#define NOISE_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Include GLM
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

/* Classe pour générer le bruit sur une fenêtre ImGui en fonction de paramètres */

class Noise {

    private:

        int noiseType; // Type de bruit
        float scale; // Echelle du bruit
        float gain; // Gain du bruit
        int octaves; // Nombre d'octaves
        float persistence; // Persistance du bruit
        float power; // UNLIMITED POWER

        bool useComputeShader; // Pour savoir si on utilise le compute shader ou vertex/fragment shader
        
        GLuint programID; 
        GLuint noiseTexture; 

        GLuint VAO, VBO // uniquement utile pour le fragment shader

    public:

        // Setters et getters des paramètres du bruit

        int getNoiseType();
        void setNoiseType(int newNoiseType);

        float getScale();
        void setScale(float newScale);

        float getGain();
        void setGain(float newGain);

        int getOctaves();
        void setOctaves(int newOctaves);

        int getPersistence();
        void setPersistence(float newPersistence);

        // Quelques Set Up 

        void setProgramID(); // chargement des ou du shader
        void initTexture(); // initialisation de la texture où sera stocker le bruit

        // Constructeur

        init();

        // Interface

        void updateInterface();



    








}