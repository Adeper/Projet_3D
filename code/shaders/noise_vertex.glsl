#version 330 core
layout(location = 0) in vec2 aPos;          // Position du sommet
layout(location = 1) in vec2 aTexCoords;     // Coordonnées de texture du sommet

out vec2 TexCoords; // Envoyer les coordonnées de texture au fragment shader

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 0.0, 1.0); // Position finale en clip space
}
