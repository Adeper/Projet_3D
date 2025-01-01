// #version 400 core

// layout(triangles, equal_spacing, cw) in;

// in vec3 tcsFragPos[]; // Depuis le TCS
// in vec2 tcsUV[];      // Reçoit les UV depuis le TCS
// out vec3 tesFragPos; // Position pour le Fragment Shader
// out vec3 tesNormal;  // Normale pour le Fragment Shader
// out vec2 tesUV;       // Coordonnées UV pour le Fragment Shader

// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;

// // Heightmap
// uniform sampler2D heightMap;    // Texture de la heightmap
// uniform float heightScale;      // Échelle des hauteurs

// vec3 computeNormal(vec2 uv) {
//     float texelSize = 1.0 / textureSize(heightMap, 0).x; // Taille d'un texel en UV
//     float hL = texture(heightMap, uv + vec2(-texelSize, 0)).r;
//     float hR = texture(heightMap, uv + vec2(texelSize, 0)).r;
//     float hD = texture(heightMap, uv + vec2(0, -texelSize)).r;
//     float hU = texture(heightMap, uv + vec2(0, texelSize)).r;
//     return normalize(vec3(hL - hR, 2.0, hD - hU));
// }


// void main() {
//     // Interpolation des positions des sommets
//     vec3 pos = gl_TessCoord.x * tcsFragPos[0] +
//                gl_TessCoord.y * tcsFragPos[1] +
//                gl_TessCoord.z * tcsFragPos[2];

//     // Interpolation des UV (comme pour les positions)
//     vec2 uv = gl_TessCoord.x * tcsUV[0] +
//             gl_TessCoord.y * tcsUV[1] +
//             gl_TessCoord.z * tcsUV[2];

//     // Ajouter la hauteur depuis la heightmap
//     float height = texture(heightMap, uv).r; // Récupère la valeur de hauteur
//     pos.y += height * heightScale;          // Applique l'échelle de hauteur

//     tesFragPos = vec3(model * vec4(pos, 1.0));
//     tesUV = uv;
//     tesNormal = computeNormal(tesUV);
 
//     gl_Position = projection * view * model * vec4(pos, 1.0);
// }
#version 430 core

layout(triangles, equal_spacing, cw) in;

in vec3 tcsFragPos[]; // Depuis le TCS
in vec2 tcsUV[];      // Reçoit les UV depuis le TCS
out vec3 tesFragPos; // Position pour le Fragment Shader
out vec3 tesNormal;  // Normale pour le Fragment Shader
out vec2 tesUV;       // Coordonnées UV pour le Fragment Shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Heightmap
uniform sampler2D heightMap;    // Texture de la heightmap
uniform float heightScale;      // Échelle des hauteurs

vec3 computeNormal(vec2 uv) {
    float texelSize = 1.0 / textureSize(heightMap, 0).x; // Taille d'un texel
    float hL = texture(heightMap, uv + vec2(-texelSize, 0)).r;
    float hR = texture(heightMap, uv + vec2(texelSize, 0)).r;
    float hD = texture(heightMap, uv + vec2(0, -texelSize)).r;
    float hU = texture(heightMap, uv + vec2(0, texelSize)).r;
    return normalize(vec3(hL - hR, 2.0, hD - hU));
}

void main() {
    // Interpolation des positions des sommets
    vec3 pos = gl_TessCoord.x * tcsFragPos[0] +
               gl_TessCoord.y * tcsFragPos[1] +
               gl_TessCoord.z * tcsFragPos[2];

    // Interpolation des UV
    vec2 uv = gl_TessCoord.x * tcsUV[0] +
              gl_TessCoord.y * tcsUV[1] +
              gl_TessCoord.z * tcsUV[2];

    // Ajouter la hauteur depuis la heightmap
    float height = texture(heightMap, uv).r; // Récupère la valeur de hauteur
    pos.y += height * heightScale;          // Applique l'échelle de hauteur

    tesFragPos = vec3(model * vec4(pos, 1.0));
    tesUV = uv;

    tesNormal = computeNormal(tesUV);

    gl_Position = projection * view * model * vec4(pos, 1.0);
}

