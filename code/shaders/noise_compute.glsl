#version 430 core

// Dimensions globales de la texture
layout (local_size_x = 16, local_size_y = 16) in;

// Texture d'image où le bruit sera généré
layout (rgba32f, binding = 0) uniform image2D noiseTexture;

// Paramètres du bruit (les mêmes que dans ton fragment shader)
uniform int noiseType;           // Type de bruit: 0 = Perlin, 1 = Simplex, 2 = Cohérent, 3 = Diamond-Square
uniform float noiseScale;        // Échelle du bruit
uniform float noiseGain;         // Gain du bruit
uniform int noiseOctaves;        // Nombre d'octaves
uniform float noisePersistence;  // Persistance du bruit
uniform float noisePower;        // Puissance du bruit
uniform int seed;                // Graine pour le bruit

// Fonction utilitaire pour générer une valeur pseudo-aléatoire
float rando(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

/* ----------- BRUIT SIMPLE ----------- */
float noise(vec2 p) {
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u * u * (3.0 - 2.0 * u);

    float res = mix(
        mix(rando(ip), rando(ip + vec2(1.0, 0.0)), u.x),
        mix(rando(ip + vec2(0.0, 1.0)), rando(ip + vec2(1.0, 1.0)), u.x), u.y
    );
    return res * res;
}

/* ----------- BRUIT SIMPLEX ----------- */
vec3 permute(vec3 x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439,
                        -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy),
                            dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

/* ----------- BRUIT COHERENT ----------- */
float coherentNoise(vec2 uv) {
    uv *= noiseScale;
    float value = sin(uv.x) * cos(uv.y);
    return (value + 1.0) * 0.5;
}

/* ----------- BRUIT DIAMOND-SQUARE ----------- */
float diamondSquare(vec2 uv) {
    vec2 p = floor(uv * noiseScale);
    vec2 offset = vec2(0.5, 0.5);
    float corners = (rando(p) + rando(p + vec2(1.0, 0.0)) +
                     rando(p + vec2(0.0, 1.0)) + rando(p + vec2(1.0, 1.0))) * 0.25;
    float edges = (rando(p + vec2(0.5, 0.0)) + rando(p + vec2(0.0, 0.5))) * 0.5;
    float center = rando(p + offset);
    return (corners + edges + center) * 0.3333;
}

/* ----------- GENERATION DE BRUIT ----------- */
float generateNoise(vec2 uv) {
    float amplitude = noiseGain;
    float frequency = noiseScale;
    float total = 0.0;

    for (int i = 0; i < noiseOctaves; i++) {
        float value;
        if (noiseType == 0) {
            value = noise(uv * frequency);
        } else if (noiseType == 1) {
            value = snoise(uv * frequency);
        } else if (noiseType == 2) {
            value = coherentNoise(uv * frequency);
        } else if (noiseType == 3) {
            value = diamondSquare(uv * frequency);
        } else {
            value = 0.0;
        }
        total += value * amplitude;
        amplitude *= noisePersistence;
        frequency *= 2.0;
    }

    total = pow(total, noisePower);
    return clamp(total, 0.0, 1.0);
}

/* ----------- MAIN ----------- */
void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy); // Identifiant global dans le groupe
    ivec2 size = imageSize(noiseTexture);       // Taille de la texture
    vec2 uv = vec2(gid) / vec2(size);           // Coordonnées normalisées

    // Ajouter un décalage basé sur la graine
    uv += vec2(float(seed) * 0.0001);

    // Générer la valeur de bruit
    float noiseValue = generateNoise(uv);

    // Stocker la valeur dans la texture
    imageStore(noiseTexture, gid, vec4(vec3(noiseValue), 1.0));
}
