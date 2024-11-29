#version 330

layout(location = 0) in vec3 position; 
layout(location = 1) in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform sampler2D heightMap;
uniform float heightScale;
uniform float lodDistance;

out vec2 fragUV;
out float distanceFactor; 

void main() {

    mat4 proj = projection;
    proj[1][1] = -proj[1][1];

    // Charger la hauteur depuis la texture
    float height = texture(heightMap, uv).r;

    // Calculer la position dans l'espace monde avec la hauteur
    vec3 pos = position;
    pos.y = height * heightScale;


    // Calculer la distance entre la caméra et le sommet
    vec3 viewPos = (view * model * vec4(pos, 1.0)).xyz;
    float distance = length(viewPos);

    // Calculer un facteur LOD basé sur la distance
    distanceFactor = clamp(1.0 - distance / lodDistance, 0.0, 1.0);

    // Ne cacher aucun sommet, tous sont visibles
    gl_Position = proj * view * model * vec4(pos, 1.0);

    fragUV = uv;
}
