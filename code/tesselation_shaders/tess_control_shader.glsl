// #version 430 core

// layout(vertices = 3) out;

// uniform float lodDistance;  // Distance maximale pour LOD
// uniform float morphDistance; // Distance pour la transition douce
// uniform vec3 cameraPosition; // Position de la caméra

// in vec3 FragPos[]; // Position des fragments depuis le Vertex Shader
// in vec2 UV[];      // Reçoit les UV du Vertex Shader
// out vec3 tcsFragPos[]; // Pour transmettre au TES
// out vec2 tcsUV[];      // Envoie les UV au TES
// out float tcsMorphFactor[]; // Envoie le morphFactor au TES

// void main() {
//     tcsFragPos[gl_InvocationID] = FragPos[gl_InvocationID];
//     tcsUV[gl_InvocationID] = UV[gl_InvocationID];

//     // Calcul de la distance au centre du patch
//     vec3 patchCenter = (FragPos[0] + FragPos[1] + FragPos[2]) / 3.0;
//     float distance = length(cameraPosition - patchCenter);

//     // Déterminer le niveau de tessellation en fonction de la distance
//     float tessLevel = 1.0; // LOD 0 (1 triangle)
//     if (distance < lodDistance * 0.33) {
//         tessLevel = 3.0; // LOD 2 (9 triangles)
//     } else if (distance < lodDistance * 0.66) {
//         tessLevel = 2.0; // LOD 1 (4 triangles)
//     }

//     // Transition douce avec morphDistance
//     float morphFactor = clamp((lodDistance - distance) / morphDistance, 0.0, 1.0);
//     float smoothedMorph = smoothstep(0.0, 1.0, morphFactor);
//     tessLevel = mix(1.0, tessLevel, smoothedMorph);

//     // Fixer les niveaux de tessellation (extérieur et intérieur)
//     gl_TessLevelOuter[0] = tessLevel;
//     gl_TessLevelOuter[1] = tessLevel;
//     gl_TessLevelOuter[2] = tessLevel;
//     gl_TessLevelInner[0] = tessLevel;

//     tcsMorphFactor[gl_InvocationID] = morphFactor;
// }
#version 430 core

layout(vertices = 3) out;

uniform float lodDistance;          // Distance maximale pour LOD
uniform float morphDistance;        // Distance pour la transition douce
uniform vec3 cameraPosition;        // Position de la caméra
uniform mat4 view;                // Matrice de vue
uniform float u_Near;               // Distance de near plane
uniform float u_Far;                // Distance de far plane
uniform float u_TessDistance;       // Distance pour définir les niveaux de tessellation

in vec3 FragPos[];                  // Position des fragments depuis le Vertex Shader
in vec2 UV[];                       // Reçoit les UV du Vertex Shader

out vec3 tcsFragPos[];              // Pour transmettre au TES
out vec2 tcsUV[];                   // Envoie les UV au TES


void main() {
    tcsFragPos[gl_InvocationID] = FragPos[gl_InvocationID];
    tcsUV[gl_InvocationID] = UV[gl_InvocationID];

    // Calcul de la position du centre du patch et conversion en espace de vue
    vec3 patchCenter = (FragPos[0] + FragPos[1] + FragPos[2]) / 3.0;
    vec4 patchCenterView = view * vec4(patchCenter, 1.0);

    // Distance au centre du patch (espace vue)
    //float distance = length(cameraPosition - vec3(patchCenterView.x, patchCenterView.y, patchCenterView.z));

    //Distance au centre du patch (espace camera)
    float distance = length(cameraPosition - patchCenter);

    // Calcul des niveaux de tessellation en fonction de la distance
    // float tessLevel = 1.0; // LOD 0 (1 triangle)
    // if (distance < lodDistance * 0.33) {
    //     tessLevel = 3.0; // LOD 2 (9 triangles)
    // } else if (distance < lodDistance * 0.66) {
    //     tessLevel = 2.0; // LOD 1 (4 triangles)
    // }

    float tessLevel = mix(4.0, 1.0, distance / lodDistance);
    tessLevel = max(tessLevel, 1.0);


    // if (patchCenterView.z < u_Near || patchCenterView.z > u_Far) {
    //     // En dehors de la vue : pas de tessellation
    //     tessLevel = 1.0;
    // } else {
    //     // Interpolation du niveau de tessellation selon la distance
    //     tessLevel = mix(1.0, 4.0, (u_Far - patchCenterView.z) / (u_Far - u_Near));
    // }


    // Fixer les niveaux de tessellation (extérieur et intérieur)
    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelInner[0] = tessLevel;
}

