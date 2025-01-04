#version 430 core

in vec3 tesFragPos; // Position du TES
in vec3 tesNormal;  // Normale du TES
in vec2 tesUV;      // Coordonnées UV depuis le TES

out vec4 color;

uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform sampler2D heightMap;   // Heightmap pour la hauteur
uniform sampler2D grassTexture; // Texture pour l'herbe
uniform sampler2D rockTexture;  // Texture pour les rochers
uniform sampler2D snowTexture;  // Texture pour la neige

uniform float grassLimit;      // Limite supérieure de l'herbe
uniform float rockLimit;       // Limite supérieure des rochers (limite inférieure pour la neige)

void main() {
    // Normalisation de la normale
    vec3 norm = normalize(tesNormal);

    // Calcul de la hauteur normale à partir de la heightmap
    float height = texture(heightMap, tesUV).r;

    // Déterminer les poids pour chaque type de texture
    float grassWeight = clamp(1.0 - height / grassLimit, 0.0, 1.0);
    float rockWeight = clamp((height - grassLimit) / (rockLimit - grassLimit), 0.0, 1.0);
    float snowWeight = clamp((height - rockLimit) / (1.0 - rockLimit), 0.0, 1.0);

    // Charger les couleurs des textures
    vec3 grassColor = texture(grassTexture, tesUV).rgb;
    vec3 rockColor = texture(rockTexture, tesUV).rgb;
    vec3 snowColor = texture(snowTexture, tesUV).rgb;

    // Mélanger les textures en fonction des poids
    vec3 blendedColor = grassColor * grassWeight +
                        rockColor * rockWeight +
                        snowColor * snowWeight;

    // Calcul de l'éclairage directionnel
    float diff = max(dot(norm, normalize(lightDirection)), 0.0);
    vec3 diffuse = diff * lightColor;

    // Appliquer l'éclairage sur la couleur mélangée
    vec3 finalColor = diffuse * blendedColor;

    color = vec4(finalColor, 1.0);
}
