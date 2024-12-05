#version 330 core

layout(triangles) in; // Entrée : 1 triangle
layout(triangle_strip, max_vertices = 27) out; // Sortie : 9 triangles (27 sommets)

in vec2 UV[]; 
in vec3 FragPos[];
in vec3 Normal[]; 
in float height[];

out vec2 outUV; // Coordonnées UV à transmettre
out vec3 outFragPos; // Position des sommets à transmettre
out vec3 outNormal;
out float outHeight;

uniform vec3 cameraPosition; // Position de la caméra
uniform float lodDistance; // Distance LOD
uniform mat4 model; // Matrice de transformation modèle
uniform mat4 view;  // Matrice de vue
uniform mat4 projection; // Matrice de projection
uniform sampler2D heightMap; // Image du bruit
uniform float heightScale;

void sendVertex(vec3 position, vec2 uv, vec3 normal){
    float textureHeight = texture(heightMap, uv).r;
    outHeight = textureHeight * heightScale;
    vec3 newPos = vec3(position.x, outHeight, position.z);

    gl_Position = projection * view * model * vec4(newPos, 1.0);
    outFragPos = newPos;
    outNormal = normal;
    outUV = uv;

    EmitVertex();
}

void main() {
    // Calcul du centre du triangle
    vec3 triCenter = (FragPos[0] + FragPos[1] + FragPos[2]) / 3.0;
    float distanceToCamera = length(cameraPosition - triCenter);

    // Calcul du facteur LOD
    float lodFactor = clamp(1.0 - distanceToCamera / lodDistance, 0.0, 1.0);

    vec3 normalA = Normal[0];
    vec3 normalB = Normal[1];
    vec3 normalC = Normal[2];

    vec2 uvA = UV[0];
    vec2 uvB = UV[1];
    vec2 uvC = UV[2];
    
    vec3 normalG = normalize((normalA + normalB + normalC) / 3.0);

    // Si suffisamment proche, on subdivise
    if (lodFactor > 0.33 && lodFactor <= 0.66) {
        // Sommets d'entrée
        vec3 A = FragPos[0];
        vec3 B = FragPos[1];
        vec3 C = FragPos[2];

        // Calcul des milieux
        vec3 M1 = (A + B) / 2.0;
        vec3 M2 = (B + C) / 2.0;
        vec3 M3 = (A + C) / 2.0;

        // Coordonnées UV des milieux
        vec2 uvM1 = (uvA + uvB) / 2.0;
        vec2 uvM2 = (uvB + uvC) / 2.0;
        vec2 uvM3 = (uvC + uvA) / 2.0;

        // Normales :
        vec3 normalM1 = normalize((normalA + normalB) / 2.0);
        vec3 normalM2 = normalize((normalB + normalC) / 2.0);
        vec3 normalM3 = normalize((normalA + normalC) / 2.0);

        // Triangle 1 : A-M1-M3
        sendVertex(A,uvA,normalA); // send A
        sendVertex(M1,uvM1,normalM1); // send M1
        sendVertex(M3,uvM3,normalM3); // send M3

        EndPrimitive();

        // Triangle 2 : M1-B-M2
        sendVertex(M1,uvM1,normalM1); // send M1
        sendVertex(B,uvB,normalB); // send B
        sendVertex(M2,uvM2,normalM2); // send M2

        EndPrimitive();

        // Triangle 3 : M3-M2-C
        sendVertex(M3,uvM3,normalM3); // send M3
        sendVertex(M2,uvM2,normalM2); // send M2
        sendVertex(C,uvC,normalC); // send C

        EndPrimitive();

        // Triangle 4 : M1-M2-M3 (centre)
        sendVertex(M1,uvM1,normalM1); // send M1
        sendVertex(M2,uvM2,normalM2); // send M2
        sendVertex(M3,uvM3,normalM3); // send M3

        EndPrimitive();
    } else if (lodFactor > 0.66) {

        vec3 A = FragPos[0];
        vec3 B = FragPos[1];
        vec3 C = FragPos[2];

        // Milieux des côtés
        vec3 M1 = B + (A - B)*2.0 / 3.0;
        vec3 M2 = B + (A - B) / 3.0;
        vec3 M3 = C + (B - C) * 2.0 / 3.0;
        vec3 M4 = C + (B - C) / 3.0;      
        vec3 M5 = A + (C - A) * 2.0 / 3.0;
        vec3 M6 = A + (C - A) / 3.0;

        // Milieux internes
        vec3 G = (A + B + C) / 3.0; // Centre/barycentre

        // UV des nouveaux points
        vec2 uvM1 = uvB + (uvA - uvB)*2.0 / 3.0;
        vec2 uvM2 = uvB + (uvA - uvB) / 3.0;
        vec2 uvM3 = uvC + (uvB - uvC)*2.0 / 3.0;
        vec2 uvM4 = uvC + (uvB - uvC) / 3.0;
        vec2 uvM5 = uvA + (uvC - uvA)*2.0 / 3.0;
        vec2 uvM6 = uvA + (uvC - uvA) / 3.0;

        vec2 uvG = (uvA + UV[1] + UV[2]) / 3.0;

        vec3 normalM1 = normalize(normalB + (normalA - normalB)*2.0 / 3.0);
        vec3 normalM2 = normalize(normalB + (normalA - normalB) / 3.0);
        vec3 normalM3 = normalize(normalC + (normalB - normalC)*2.0 / 3.0);
        vec3 normalM4 = normalize(normalC + (normalB - normalC) / 3.0);
        vec3 normalM5 = normalize(normalA + (normalC - normalA)*2.0 / 3.0);
        vec3 normalM6 = normalize(normalA + (normalC - normalA) / 3.0);

        // Triangle 1 : A-M1-M6
        sendVertex(A,uvA,normalA); // send A
        sendVertex(M1,uvM1,normalM1); // send M1
        sendVertex(M6,uvM6,normalM6); // send M6

        EndPrimitive();

        // Triangle 2 : M1-M2-G
        sendVertex(M1,uvM1,normalM1); // send M1
        sendVertex(M2,uvM2,normalM2); // send M2
        sendVertex(G,uvG,normalG); // send G

        EndPrimitive();

        // Triangle 3 : M6-G-M5
        sendVertex(M6,uvM6,normalM6); // send M6
        sendVertex(G,uvG,normalG); // send G
        sendVertex(M5,uvM5,normalM5); // send M5

        EndPrimitive();

        // Triangle 4 : M1-M6-G
        sendVertex(M1,uvM1,normalM1); // send M1
        sendVertex(M6,uvM6,normalM6); // send M6
        sendVertex(G,uvG,normalG); // send G

        EndPrimitive();

        // Triangle 5 : M2-B-M3
        sendVertex(M2,uvM2,normalM2); // send M2
        sendVertex(B,uvB,normalB); // send B
        sendVertex(M3,uvM3,normalM3); // send M3

        EndPrimitive();

        // Triangle 6 : G-M3-M4
        sendVertex(G,uvG,normalG); // send G
        sendVertex(M3,uvM3,normalM3); // send M3
        sendVertex(M4,uvM4,normalM4); // send M4

        EndPrimitive();

        // Triangle 7 : M2-G-M3
        sendVertex(M2,uvM2,normalM2); // send M2
        sendVertex(G,uvG,normalG); // send G
        sendVertex(M3,uvM3,normalM3); // send M3

        EndPrimitive();

        // Triangle 8 : M5-M4-C
        sendVertex(M5,uvM5,normalM5); // send M5
        sendVertex(M4,uvM4,normalM4); // send M4
        sendVertex(C,uvC,normalC); // send C

        EndPrimitive();

        // Triangle 9 : G-M5-M4
        sendVertex(G,uvG,normalG); // send G
        sendVertex(M5,uvM5,normalM5); // send M5
        sendVertex(M4,uvM4,normalM4); // send M4

        EndPrimitive();
        
    } else {
        // Pas de subdivision : transmettre les sommets d'origine
        for (int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position;
            outFragPos = FragPos[i];
            outNormal = Normal[i];
            outUV = UV[i];
            outHeight = height[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}