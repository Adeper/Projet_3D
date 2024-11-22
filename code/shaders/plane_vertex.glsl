#version 330 core

layout(location = 0) in vec3 inPosition; // Position du sommet
layout(location = 1) in vec3 inNormal;   // Normale du sommet

out vec3 fragNormal; // Normale interpolée
out vec3 fragPosition; // Position interpolée

uniform mat4 model;      // Matrice de transformation
uniform mat4 view;       // Matrice de vue
uniform mat4 projection; // Matrice de projection

void main() {
    fragNormal = mat3(transpose(inverse(model))) * inNormal; // Transforme la normale
    fragPosition = vec3(model * vec4(inPosition, 1.0));      // Transforme la position
    gl_Position = projection * view * vec4(fragPosition, 1.0);
}
