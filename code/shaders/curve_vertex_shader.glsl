#version 330 core

layout(location = 0) in vec3 position;

out vec2 UV; // Coordonnées UV

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    UV = vec2(position.x, position.z); // Exemple simple de génération de UV
}
