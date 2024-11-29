#version 330

in vec2 fragUV;
in float distanceFactor; // Reçu depuis le vertex shader

uniform sampler2D heightMap; // Texture de hauteur
uniform float heightScale;   // Échelle de hauteur

out vec4 fragColor;

void main() {
    
    float height = texture(heightMap, fragUV).r * heightScale;

    
    vec3 color;
    if (distanceFactor > 0.66) {
        color = vec3(0.0, 1.0, 0.0); // Vert (proche, LOD élevé)
    } else if (distanceFactor > 0.33) {
        color = vec3(1.0, 1.0, 0.0); // Jaune (moyen, LOD intermédiaire)
    } else {
        color = vec3(1.0, 0.0, 0.0); // Rouge (loin, LOD faible)
    }

    fragColor = vec4(color * (0.5 + height * 0.5), 1.0);
}
