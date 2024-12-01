#version 330 core

in vec2 UV;           // Coordonnées outUV
in vec3 FragPos;      // Position du fragment dans l'espace monde  

in float height;      // Hauteur interpolée depuis le vertex shader

out vec4 color;

uniform sampler2D grassTexture;  // Texture pour les herbes
// uniform sampler2D rockTexture;   // Texture pour les rochers
// uniform sampler2D snowTexture;   // Texture pour la neige

// in vec3 Normal;
// uniform vec3 LightPosition_worldspace;
// uniform vec3 LightColor_worldspace;
// uniform vec3 color_Mesh;

// uniform float grassLimit;
// uniform float rockLimit;

void main() {

    // vec3 norm = normalize(Normal);

    // vec3 lightDir = normalize(LightPosition_worldspace - FragPos);

    // float diff = max(dot(norm, lightDir), 0.0);

    // vec3 diffuse = diff * LightColor_worldspace;

    // vec3 result = diffuse * color_Mesh;

    // vec4 grassColor = texture(grassTexture, UV);
    // vec4 rockColor = texture(rockTexture, UV);
    // vec4 snowColor = texture(snowTexture, UV);
    
    // vec4 terrainColor;
    // if (height < grassLimit) {
    //     terrainColor = grassColor;
    // } else if (height < rockLimit) {
    //     float t = (height - grassLimit) / (rockLimit - grassLimit);
    //     terrainColor = mix(grassColor, rockColor, t);
    // } else {
    //     float t = (height - rockLimit) / (1.0 - rockLimit);
    //     terrainColor = mix(rockColor, snowColor, t);
    // }

    // color = terrainColor;

    color = vec4(mix(vec3(0.2,0.5,0.8), vec3(0.8,0.6,0.4), height), 1.0);
    // color = texture(grassTexture, UV);
}
