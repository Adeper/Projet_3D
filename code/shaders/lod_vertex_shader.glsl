#version 330

layout(location = 0) in vec3 position; 
layout(location = 1) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float lodDistance; // Distance de LOD pour ajuster les détails

out vec2 fragUV;
out float distanceFactor; // Facteur de distance pour LOD

void main() {
    // Calcul de la distance normalisée entre la caméra et le vertex
    float distance = length((view * model * vec4(position, 1.0)).xyz);
    distanceFactor = clamp(1.0 - distance / lodDistance, 0.0, 1.0);

    // Ajustement de la position en fonction du LOD
    vec3 adjustedPosition = position;
    adjustedPosition.y *= distanceFactor;

    gl_Position = projection * view * model * vec4(adjustedPosition, 1.0);
    fragUV = uv;
}
