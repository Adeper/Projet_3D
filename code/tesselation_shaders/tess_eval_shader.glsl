#version 430 core

layout(triangles, equal_spacing, cw) in;

in vec3 tcsFragPos[]; // Depuis le TCS
in vec2 tcsUV[];      // Reçoit les UV depuis le TCS
in vec3 tcsNormal[];  // Reçoit les normales depuis le TCS
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

// Tentative : Utiliser PN Triangle pour lisser les normales
vec3 computePNPosition(vec3 p0, vec3 p1, vec3 p2, vec3 n0, vec3 n1, vec3 n2, vec3 baryCoord) {
    // Positions moyennes
    vec3 averagePos = p0 * baryCoord.x + p1 * baryCoord.y + p2 * baryCoord.z;

    // Normales moyennes
    vec3 averageNormal = normalize(n0 * baryCoord.x + n1 * baryCoord.y + n2 * baryCoord.z);

    // Ajustement de la position avec la normale
    vec3 adjustedPos = averagePos + averageNormal; // Facteur pour l'effet de lissage

    return adjustedPos;
}

vec3 morphEdge(vec3 posA, vec3 posB, float factor) {
    return mix(posA, posB, factor);
}

void main() {
    // Récupération des barycentriques (gl_TessCoord)
    vec3 baryCoord = gl_TessCoord;

    // // Interpolation des positions des sommets
    // vec3 pos = gl_TessCoord.x * tcsFragPos[0] +
    //            gl_TessCoord.y * tcsFragPos[1] +
    //            gl_TessCoord.z * tcsFragPos[2];

    // Calcul de la position interpolée (PN Triangle)
    vec3 pnPos = computePNPosition(
        tcsFragPos[0], tcsFragPos[1], tcsFragPos[2],
        tcsNormal[0], tcsNormal[1], tcsNormal[2],
        baryCoord
    );

    // Interpolation des UV
    vec2 uv = gl_TessCoord.x * tcsUV[0] +
              gl_TessCoord.y * tcsUV[1] +
              gl_TessCoord.z * tcsUV[2];

    // Ajouter la hauteur depuis la heightmap
    float height = texture(heightMap, uv).r; // Récupère la valeur de hauteur
    // pos.y += height * heightScale;          // Applique l'échelle de hauteur
    pnPos.y += height * heightScale;        // Applique l'échelle de hauteur

    // tesFragPos = vec3(model * vec4(pos, 1.0));

    tesFragPos = vec3(model * vec4(pnPos, 1.0));
    tesUV = uv;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    tesNormal = normalize(normalMatrix * computeNormal(uv));

    gl_Position = projection * view * model * vec4(pnPos, 1.0);
}

