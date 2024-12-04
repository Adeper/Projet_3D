#version 330 core

layout(triangles) in; // Entrée : 1 triangle
layout(triangle_strip, max_vertices = 27) out; // Sortie : 9 triangles (27 sommets)

in vec2 UV[]; 
in vec3 FragPos[]; 
in float height[];

out vec2 outUV; // Coordonnées UV à transmettre
out vec3 outFragPos; // Position des sommets à transmettre
out float outHeight;

uniform vec3 cameraPosition; // Position de la caméra
uniform float lodDistance; // Distance LOD
uniform mat4 model; // Matrice de transformation modèle
uniform mat4 view;  // Matrice de vue
uniform mat4 projection; // Matrice de projection

void main() {
    // Calcul du centre du triangle
    vec3 triCenter = (FragPos[0] + FragPos[1] + FragPos[2]) / 3.0;
    float distanceToCamera = length(cameraPosition - triCenter);

    // Calcul du facteur LOD
    float lodFactor = clamp(1.0 - distanceToCamera / lodDistance, 0.0, 1.0);

    // Hauteur:
    float heightA = height[0];
    float heightB = height[1];
    float heightC = height[2];

    float heightM1 = (height[0] + height[1]) / 2.0;
    float heightM2 = (height[1] + height[2]) / 2.0;
    float heightM3 = (height[0] + height[2]) / 2.0;

    float heightG = (height[0] + height[1] + height[2]) / 3.0;

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
        vec2 uvM1 = (UV[0] + UV[1]) / 2.0;
        vec2 uvM2 = (UV[1] + UV[2]) / 2.0;
        vec2 uvM3 = (UV[2] + UV[0]) / 2.0;

        // Triangle 1 : A-M1-M3
        gl_Position = projection * view * model * vec4(A, 1.0); // A
        outFragPos = A;
        outUV = UV[0];
        outHeight = heightA;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M1, 1.0); // M1
        outFragPos = M1;
        outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M3, 1.0); // M3
        outFragPos = M3;
        outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        EndPrimitive();

        // Triangle 2 : M1-B-M2
        gl_Position = projection * view * model * vec4(M1, 1.0); // M1
        outFragPos = M1;
        outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(B, 1.0); // B
        outFragPos = B;
        outHeight = heightB;
        outUV = UV[1];
        EmitVertex();

        gl_Position = projection * view * model * vec4(M2, 1.0); // M2
        outFragPos = M2;
        outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        EndPrimitive();

        // Triangle 3 : M3-M2-C
        gl_Position = projection * view * model * vec4(M3, 1.0); // M3
        outFragPos = M3;
        outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M2, 1.0); // M2
        outFragPos = M2;
        outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        gl_Position = projection * view * model * vec4(C, 1.0); // C
        outFragPos = C;
        outUV = UV[2];
        EmitVertex();

        EndPrimitive();

        // Triangle 4 : M1-M2-M3 (centre)
        gl_Position = projection * view * model * vec4(M1, 1.0); // M1
        outFragPos = M1;
        outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M2, 1.0); // M2
        outFragPos = M2;
        outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M3, 1.0); // M3
        outFragPos = M3;
        outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        EndPrimitive();
    } else if (lodFactor > 0.66) {

        vec3 A = FragPos[0];
        vec3 B = FragPos[1];
        vec3 C = FragPos[2];

        // Milieux des côtés
        vec3 M1 = (A + B) / 2.0;
        vec3 M2 = (B + C) / 2.0;
        vec3 M3 = (A + C) / 2.0;

        // Milieux internes
        vec3 G = (A + B + C) / 3.0; // Centre/barycentre

        // UV des nouveaux points
        vec2 uvM1 = (UV[0] + UV[1]) / 2.0;
        vec2 uvM2 = (UV[1] + UV[2]) / 2.0;
        vec2 uvM3 = (UV[2] + UV[0]) / 2.0;
        vec2 uvG = (UV[0] + UV[1] + UV[2]) / 3.0;

        // Triangle 1 : A-M1-G
        gl_Position = projection * view * model * vec4(A, 1.0);
        outFragPos = A; outUV = UV[0];
        outHeight = heightA;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M1, 1.0);
        outFragPos = M1; outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 2 : M1-B-G
        gl_Position = projection * view * model * vec4(M1, 1.0);
        outFragPos = M1; outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(B, 1.0);
        outFragPos = B; outUV = UV[1];
        outHeight = heightB;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 3 : B-M2-G
        gl_Position = projection * view * model * vec4(B, 1.0);
        outFragPos = B; outUV = UV[1];
        outHeight = heightB;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M2, 1.0);
        outFragPos = M2; outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 4 : M2-C-G
        gl_Position = projection * view * model * vec4(M2, 1.0);
        outFragPos = M2; outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        gl_Position = projection * view * model * vec4(C, 1.0);
        outFragPos = C; outUV = UV[2];
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 5 : C-M3-G
        gl_Position = projection * view * model * vec4(C, 1.0);
        outFragPos = C; outUV = UV[2];
        EmitVertex();

        gl_Position = projection * view * model * vec4(M3, 1.0);
        outFragPos = M3; outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 6 : M3-A-G
        gl_Position = projection * view * model * vec4(M3, 1.0);
        outFragPos = M3; outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        gl_Position = projection * view * model * vec4(A, 1.0);
        outFragPos = A; outUV = UV[0];
        outHeight = heightA;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 7 : M1-M3-G
        gl_Position = projection * view * model * vec4(M1, 1.0);
        outFragPos = M1; outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M3, 1.0);
        outFragPos = M3; outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 8 : M1-M2-G
        gl_Position = projection * view * model * vec4(M1, 1.0);
        outFragPos = M1; outUV = uvM1;
        outHeight = heightM1;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M2, 1.0);
        outFragPos = M2; outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();
        EndPrimitive();

        // Triangle 9 : M2-M3-G
        gl_Position = projection * view * model * vec4(M2, 1.0);
        outFragPos = M2; outUV = uvM2;
        outHeight = heightM2;
        EmitVertex();

        gl_Position = projection * view * model * vec4(M3, 1.0);
        outFragPos = M3; outUV = uvM3;
        outHeight = heightM3;
        EmitVertex();

        gl_Position = projection * view * model * vec4(G, 1.0);
        outFragPos = G; outUV = uvG;
        outHeight = heightG;
        EmitVertex();

        EndPrimitive();
        
    } else {
        // Pas de subdivision : transmettre les sommets d'origine
        for (int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position;
            outFragPos = FragPos[i];
            outUV = UV[i];
            outHeight = height[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}