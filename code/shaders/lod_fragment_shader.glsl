#version 330

in vec2 fragUV;
in float distanceFactor; // Reçu depuis le vertex shader

uniform sampler2D heightMap; // Texture de hauteur
uniform float heightScale;   // Échelle de hauteur

out vec4 fragColor;

void main() {
    // Récupération de la hauteur depuis la texture
    float height = texture(heightMap, fragUV).r * heightScale;

    // Définition des couleurs selon les niveaux de détail
    vec3 color;
    if (distanceFactor > 0.66) {
        color = vec3(0.0, 1.0, 0.0); // Vert (LOD élevé)
    } else if (distanceFactor > 0.33) {
        color = vec3(1.0, 1.0, 0.0); // Jaune (LOD moyen)
    } else {
        color = vec3(1.0, 0.0, 0.0); // Rouge (LOD faible)
    }

    // Appliquer la hauteur sur la teinte
    fragColor = vec4(color * height, 1.0);
}
