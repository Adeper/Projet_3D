#version 330 core

in vec3 FragPos;
in vec2 outUV;           // Coordonnées outUV
in vec3 Normal;
in float outHeight;      // Hauteur interpolée depuis le vertex shader

out vec4 FragColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform vec3 ambientColor;
uniform vec3 color_Mesh;

uniform sampler2D heightMap;
uniform sampler2D grassTexture;  // Texture pour les herbes
uniform sampler2D rockTexture;   // Texture pour les rochers
uniform sampler2D snowTexture;   // Texture pour la neige

uniform float heightScale;

uniform float grassLimit;
uniform float rockLimit;

void main() {

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    // Composante diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Composante ambiante
    vec3 ambient = ambientColor * lightColor;

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

    vec3 result = (ambient + diffuse) * color_Mesh;
    FragColor = terrainColor * vec4(result, 1.0);
}
