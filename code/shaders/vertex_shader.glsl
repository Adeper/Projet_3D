#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 aNormal;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap;
uniform float heightScale;
uniform vec3 cameraPosition; 
uniform float lodDistance; 

void main() {
    float height = texture(heightMap, vertexUV).r;

    // Calculer la distance entre le sommet et la caméra
    float distanceToCamera = length(cameraPosition - (vec3(model * vec4(vertexPosition, 1.0))));

    // Calculer le facteur LOD (1.0 proche, 0.0 éloigné)
    float lodFactor = clamp(1.0 - distanceToCamera / lodDistance, 0.0, 1.0);

    // Appliquer l'échelle de hauteur avec le LOD
    vec3 adjustedPosition = vertexPosition;
    adjustedPosition.y = height * heightScale * lodFactor;

    // Calculer la position finale du sommet
    gl_Position = projection * view * model * vec4(adjustedPosition, 1.0);

    // Transmettre les valeurs aux autres shaders
    FragPos = vec3(model * vec4(adjustedPosition, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    UV = vertexUV;
}
