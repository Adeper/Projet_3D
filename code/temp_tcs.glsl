#version 400 core

layout(vertices = 3) out;

uniform float lodDistance;  // Distance maximum pour LOD
uniform float morphDistance; // Distance pour la transition douce
uniform vec3 cameraPosition; // Position de la caméra

in vec3 FragPos[]; // Position des fragments depuis le Vertex Shader
in vec2 UV[];      // Reçoit les UV du Vertex Shader
out vec3 tcsFragPos[]; // Pour transmettre au TES
out vec2 tcsUV[];      // Envoie les UV au TES

void main() {
    tcsFragPos[gl_InvocationID] = FragPos[gl_InvocationID];
    tcsUV[gl_InvocationID] = UV[gl_InvocationID];

    // Calcul de la distance au centre du patch
    vec3 patchCenter = (FragPos[0] + FragPos[1] + FragPos[2]) / 3.0;
    float distance = length(cameraPosition - patchCenter);

    // Calcul du morphFactor
    float morphFactor = normalize(clamp((lodDistance - distance) / morphDistance, 0.0, 1.0));

    // Niveaux de LOD avec transitions douces
    float tessLevel = 1.0;
    if (distance < lodDistance * 0.33) {
        tessLevel = mix(1.0, 6.0, morphFactor);
    } else if (distance < lodDistance * 0.66) {
        tessLevel = mix(1.0, 3.0, morphFactor);
    }

    // Assurer la cohérence des bords
    gl_TessLevelOuter[0] = tessLevel; // Entre les sommets 0 et 1
    gl_TessLevelOuter[1] = tessLevel; // Entre les sommets 1 et 2
    gl_TessLevelOuter[2] = tessLevel; // Entre les sommets 2 et 0
    gl_TessLevelInner[0] = tessLevel; // Centre du patch
}
