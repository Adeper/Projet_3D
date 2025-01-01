#version 330 core

layout(triangles) in; // Entrée : 1 triangle
layout(triangle_strip, max_vertices = 45) out; // Sortie : stitching inclus (15 triangles)

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
    // Sommets d'entrée
    vec3 A = FragPos[0];
    vec3 B = FragPos[1];
    vec3 C = FragPos[2];

    vec2 uvA = UV[0];
    vec2 uvB = UV[1];
    vec2 uvC = UV[2];

    vec3 normalA = Normal[0];
    vec3 normalB = Normal[1];
    vec3 normalC = Normal[2];

    // Calcul des milieux du triangle LOD Faible
    vec3 M1_prime = (A + B) / 2.0;
    vec3 M2_prime = (B + C) / 2.0;
    vec3 M3_prime = (C + A) / 2.0;

    vec2 uvM1_prime = (uvA + uvB) / 2.0;
    vec2 uvM2_prime = (uvB + uvC) / 2.0;
    vec2 uvM3_prime = (uvC + uvA) / 2.0;

    vec3 normalM1_prime = normalize((normalA + normalB) / 2.0);
    vec3 normalM2_prime = normalize((normalB + normalC) / 2.0);
    vec3 normalM3_prime = normalize((normalC + normalA) / 2.0);

    // Calcul des milieux du triangle LOD Moyen
    vec3 M1 = (A + M1_prime) / 2.0;
    vec3 M2 = (B + M2_prime) / 2.0;
    vec3 M3 = (C + M3_prime) / 2.0;

    vec2 uvM1 = (uvA + uvM1_prime) / 2.0;
    vec2 uvM2 = (uvB + uvM2_prime) / 2.0;
    vec2 uvM3 = (uvC + uvM3_prime) / 2.0;

    vec3 normalM1 = normalize((normalA + normalM1_prime) / 2.0);
    vec3 normalM2 = normalize((normalB + normalM2_prime) / 2.0);
    vec3 normalM3 = normalize((normalC + normalM3_prime) / 2.0);

    // Stitching - Connexion entre LOD Faible et LOD Moyen

    // Triangle 1 : A-M1'-M1
    sendVertex(A, uvA, normalA);
    sendVertex(M1_prime, uvM1_prime, normalM1_prime);
    sendVertex(M1, uvM1, normalM1);
    EndPrimitive();

    // Triangle 2 : M1'-M1-B
    sendVertex(M1_prime, uvM1_prime, normalM1_prime);
    sendVertex(M1, uvM1, normalM1);
    sendVertex(B, uvB, normalB);
    EndPrimitive();

    // Triangle 3 : B-M2'-M2
    sendVertex(B, uvB, normalB);
    sendVertex(M2_prime, uvM2_prime, normalM2_prime);
    sendVertex(M2, uvM2, normalM2);
    EndPrimitive();

    // Triangle 4 : M2'-M2-C
    sendVertex(M2_prime, uvM2_prime, normalM2_prime);
    sendVertex(M2, uvM2, normalM2);
    sendVertex(C, uvC, normalC);
    EndPrimitive();

    // Triangle 5 : C-M3'-M3
    sendVertex(C, uvC, normalC);
    sendVertex(M3_prime, uvM3_prime, normalM3_prime);
    sendVertex(M3, uvM3, normalM3);
    EndPrimitive();

    // Triangle 6 : M3'-M3-A
    sendVertex(M3_prime, uvM3_prime, normalM3_prime);
    sendVertex(M3, uvM3, normalM3);
    sendVertex(A, uvA, normalA);
    EndPrimitive();
}


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

    // Sommets d'entrée
    vec3 A = FragPos[0];
    vec3 B = FragPos[1];
    vec3 C = FragPos[2];

    // Calcul des milieux des arêtes
    vec3 M1 = (A + B) / 2.0;
    vec3 M2 = (B + C) / 2.0;
    vec3 M3 = (A + C) / 2.0;

    vec2 uvM1 = (uvA + uvB) / 2.0;
    vec2 uvM2 = (uvB + uvC) / 2.0;
    vec2 uvM3 = (uvC + uvA) / 2.0;

    vec3 normalM1 = normalize((normalA + normalB) / 2.0);
    vec3 normalM2 = normalize((normalB + normalC) / 2.0);
    vec3 normalM3 = normalize((normalA + normalC) / 2.0);


}
