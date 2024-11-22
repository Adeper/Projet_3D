#version 330 core

in vec3 fragNormal;    // Normale interpolée
in vec3 fragPosition;  // Position interpolée

out vec4 fragColor; // Couleur finale

uniform vec3 lightDir = normalize(vec3(1.0, -1.0, -1.0)); // Direction de la lumière
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);            // Couleur de la lumière
uniform vec3 objectColor = vec3(0.6, 0.8, 1.0);           // Couleur de l'objet

void main() {
    float ambient = 0.2; // Lumière ambiante
    float diffuse = max(dot(normalize(fragNormal), -lightDir), 0.0); // Diffuse
    vec3 color = (ambient + diffuse) * lightColor * objectColor;
    fragColor = vec4(color, 1.0);
}
