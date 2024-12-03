#version 330 core

in vec2 outUV;           // Coordonnées outUV
in vec3 FragPos;      // Position du fragment dans l'espace monde  

in float outHeight;      // Hauteur interpolée depuis le vertex shader

out vec4 color;


uniform sampler2D heightMap;
uniform sampler2D grassTexture;  // Texture pour les herbes
uniform sampler2D rockTexture;   // Texture pour les rochers
uniform sampler2D snowTexture;   // Texture pour la neige

uniform float heightScale;

uniform float grassLimit;
uniform float rockLimit;

void main() {


    vec4 grassColor = texture(grassTexture, outUV);
    vec4 rockColor = texture(rockTexture, outUV);
    vec4 snowColor = texture(snowTexture, outUV);
    
    vec4 terrainColor;
    if (outHeight < grassLimit * heightScale) {
        terrainColor = grassColor;
    } else if (outHeight < rockLimit * heightScale) {
        float t = ((outHeight/heightScale) - grassLimit) / (rockLimit - grassLimit);
        terrainColor = mix(grassColor, rockColor, t);
    } else {
        float t = ((outHeight/heightScale) - rockLimit) / (1.0 - rockLimit);
        terrainColor = mix(rockColor, snowColor, t);
    }

    color = terrainColor;

}
