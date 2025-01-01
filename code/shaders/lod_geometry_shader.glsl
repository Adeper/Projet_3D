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

vec3 computeMidpoint(vec3 P1, vec3 P2, vec2 uv1, vec2 uv2) {
    vec2 uvMid = (uv1 + uv2) / 2.0;
    float heightMid = texture(heightMap, uvMid).r * heightScale;
    return vec3((P1.x + P2.x) / 2.0, heightMid, (P1.z + P2.z) / 2.0);
}

vec3 computeNormal(vec2 uv) {

    float texelSize = 1.0 / textureSize(heightMap, 0).x;

    float hL = texture(heightMap, uv + vec2(-texelSize, 0)).r * heightScale;
    float hR = texture(heightMap, uv + vec2(texelSize, 0)).r * heightScale;
    float hD = texture(heightMap, uv + vec2(0, -texelSize)).r * heightScale;
    float hU = texture(heightMap, uv + vec2(0, texelSize)).r * heightScale;

    vec3 normal = normalize(vec3(hL - hR, 2.0, hD - hU));
    return normal;
}

void createTransitionStrip(vec3 A, vec3 B, vec2 uvA, vec2 uvB) {
    vec3 M = computeMidpoint(A, B, uvA, uvB);
    vec3 normalM = computeNormal((uvA + uvB) / 2.0);

    sendVertex(M, (uvA + uvB) / 2.0, normalM);
    sendVertex(B, uvB, computeNormal(uvB));
    sendVertex(A, uvA, computeNormal(uvA));
    EndPrimitive();
}


void main() {

    vec3 A = FragPos[0];
    vec3 B = FragPos[1];
    vec3 C = FragPos[2];
    vec2 uvA = UV[0];
    vec2 uvB = UV[1];
    vec2 uvC = UV[2];

    float distanceToCamera = length(cameraPosition - (A + B + C) / 3.0);
    float lodFactor = clamp(1.0 - distanceToCamera / lodDistance, 0.0, 1.0);

    if (lodFactor > 0.33 && lodFactor <= 0.66) {
        vec3 M1 = computeMidpoint(A, B, uvA, uvB);
        vec3 M2 = computeMidpoint(B, C, uvB, uvC);
        vec3 M3 = computeMidpoint(C, A, uvC, uvA);

        // Triangles du LOD moyen
        sendVertex(A, uvA, computeNormal(uvA));
        sendVertex(M1, (uvA + uvB) / 2.0, computeNormal((uvA + uvB) / 2.0));
        sendVertex(M3, (uvA + uvC) / 2.0, computeNormal((uvA + uvC) / 2.0));
        EndPrimitive();

        sendVertex(M1, (uvA + uvB) / 2.0, computeNormal((uvA + uvB) / 2.0));
        sendVertex(B, uvB, computeNormal(uvB));
        sendVertex(M2, (uvB + uvC) / 2.0, computeNormal((uvB + uvC) / 2.0));
        EndPrimitive();

        sendVertex(M3, (uvA + uvC) / 2.0, computeNormal((uvA + uvC) / 2.0));
        sendVertex(M2, (uvB + uvC) / 2.0, computeNormal((uvB + uvC) / 2.0));
        sendVertex(C, uvC, computeNormal(uvC));
        EndPrimitive();

        sendVertex(M1, (uvA + uvB) / 2.0, computeNormal((uvA + uvB) / 2.0));
        sendVertex(M2, (uvB + uvC) / 2.0, computeNormal((uvB + uvC) / 2.0));
        sendVertex(M3, (uvA + uvC) / 2.0, computeNormal((uvA + uvC) / 2.0));
        EndPrimitive();

        // Bandes de transition
        createTransitionStrip(A, B, uvA, uvB);
        createTransitionStrip(B, C, uvB, uvC);
        createTransitionStrip(C, A, uvC, uvA);
        
    // } else if (lodFactor > 0.66) {

    //     vec3 A = FragPos[0];
    //     vec3 B = FragPos[1];
    //     vec3 C = FragPos[2];

    //     // Milieux des côtés
    //     vec3 M1 = B + (A - B)*2.0 / 3.0;
    //     vec3 M2 = B + (A - B) / 3.0;
    //     vec3 M3 = C + (B - C) * 2.0 / 3.0;
    //     vec3 M4 = C + (B - C) / 3.0;      
    //     vec3 M5 = A + (C - A) * 2.0 / 3.0;
    //     vec3 M6 = A + (C - A) / 3.0;

    //     // Milieux internes
    //     vec3 G = (A + B + C) / 3.0; // Centre/barycentre

    //     // UV des nouveaux points
    //     vec2 uvM1 = uvB + (uvA - uvB)*2.0 / 3.0;
    //     vec2 uvM2 = uvB + (uvA - uvB) / 3.0;
    //     vec2 uvM3 = uvC + (uvB - uvC)*2.0 / 3.0;
    //     vec2 uvM4 = uvC + (uvB - uvC) / 3.0;
    //     vec2 uvM5 = uvA + (uvC - uvA)*2.0 / 3.0;
    //     vec2 uvM6 = uvA + (uvC - uvA) / 3.0;

    //     vec2 uvG = (uvA + UV[1] + UV[2]) / 3.0;

    //     vec3 normalM1 = normalize(normalB + (normalA - normalB)*2.0 / 3.0);
    //     vec3 normalM2 = normalize(normalB + (normalA - normalB) / 3.0);
    //     vec3 normalM3 = normalize(normalC + (normalB - normalC)*2.0 / 3.0);
    //     vec3 normalM4 = normalize(normalC + (normalB - normalC) / 3.0);
    //     vec3 normalM5 = normalize(normalA + (normalC - normalA)*2.0 / 3.0);
    //     vec3 normalM6 = normalize(normalA + (normalC - normalA) / 3.0);

    //     // Triangle 1 : A-M1-M6
    //     sendVertex(A,uvA,normalA); // send A
    //     sendVertex(M1,uvM1,normalM1); // send M1
    //     sendVertex(M6,uvM6,normalM6); // send M6

    //     EndPrimitive();

    //     // Triangle 2 : M1-M2-G
    //     sendVertex(M1,uvM1,normalM1); // send M1
    //     sendVertex(M2,uvM2,normalM2); // send M2
    //     sendVertex(G,uvG,normalG); // send G

    //     EndPrimitive();

    //     // Triangle 3 : M6-G-M5
    //     sendVertex(M6,uvM6,normalM6); // send M6
    //     sendVertex(G,uvG,normalG); // send G
    //     sendVertex(M5,uvM5,normalM5); // send M5

    //     EndPrimitive();

    //     // Triangle 4 : M1-M6-G
    //     sendVertex(M1,uvM1,normalM1); // send M1
    //     sendVertex(M6,uvM6,normalM6); // send M6
    //     sendVertex(G,uvG,normalG); // send G

    //     EndPrimitive();

    //     // Triangle 5 : M2-B-M3
    //     sendVertex(M2,uvM2,normalM2); // send M2
    //     sendVertex(B,uvB,normalB); // send B
    //     sendVertex(M3,uvM3,normalM3); // send M3

    //     EndPrimitive();

    //     // Triangle 6 : G-M3-M4
    //     sendVertex(G,uvG,normalG); // send G
    //     sendVertex(M3,uvM3,normalM3); // send M3
    //     sendVertex(M4,uvM4,normalM4); // send M4

    //     EndPrimitive();

    //     // Triangle 7 : M2-G-M3
    //     sendVertex(M2,uvM2,normalM2); // send M2
    //     sendVertex(G,uvG,normalG); // send G
    //     sendVertex(M3,uvM3,normalM3); // send M3

    //     EndPrimitive();

    //     // Triangle 8 : M5-M4-C
    //     sendVertex(M5,uvM5,normalM5); // send M5
    //     sendVertex(M4,uvM4,normalM4); // send M4
    //     sendVertex(C,uvC,normalC); // send C

    //     EndPrimitive();

    //     // Triangle 9 : G-M5-M4
    //     sendVertex(G,uvG,normalG); // send G
    //     sendVertex(M5,uvM5,normalM5); // send M5
    //     sendVertex(M4,uvM4,normalM4); // send M4

    //     EndPrimitive();
        
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