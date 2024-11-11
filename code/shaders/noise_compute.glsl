#version 430 core

// Dimensions de la texture (spécifiées par l'application OpenGL)
layout(local_size_x = 16, local_size_y = 16) in; // Taille du groupe de travail

// Texture de sortie
layout(rgba32f, binding = 0) uniform image2D outputImage;

// Paramètres du bruit
uniform int noiseType;           // Type de bruit: 0 = Perlin, 1 = Simplex, etc.
uniform float noiseScale;         // Échelle du bruit
uniform float noiseGain;          // Gain du bruit
uniform int noiseOctaves;         // Nombre d'octaves
uniform float noisePersistence;   // Persistance du bruit
uniform float noisePower;         // Puissance du bruit
uniform int seed;                 // Graine pour le bruit

// Fonction de hachage
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float rando(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 ip = floor(p);
    vec2 u = fract(p);
    u = u * u * (3.0 - 2.0 * u);

    float res = mix(
        mix(rando(ip), rando(ip + vec2(1.0, 0.0)), u.x),
        mix(rando(ip + vec2(0.0, 1.0)), rando(ip + vec2(1.0, 1.0)), u.x),
        u.y);
    return res * res;
}

// Génération de bruit
float generateNoise(vec2 uv) {
    float amplitude = noiseGain;    // Gain initial
    float frequency = noiseScale;  // Échelle initiale
    float total = 0.0;             // Accumulateur de bruit

    for (int i = 0; i < noiseOctaves; i++) {
        total += noise(uv * frequency) * amplitude; // Contribution de chaque octave
        amplitude *= noisePersistence;             // Diminuer l'amplitude
        frequency *= 2.0;                          // Doubler la fréquence
    }

    // Appliquer la puissance pour ajuster la courbe
    total = pow(total, noisePower);

    // Normaliser (optionnel)
    total = clamp(total, 0.0, 1.0);
    return total;
}

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy); // Position actuelle du thread
    ivec2 imageSize = imageSize(outputImage);          // Taille de l'image
    vec2 uv = vec2(pixelCoord) / vec2(imageSize);      // Normalisation des coordonnées [0,1]

    // Calcul du bruit
    float noiseValue = generateNoise(uv);

    // Écriture dans la texture de sortie
    vec4 color = vec4(vec3(noiseValue), 1.0); // Valeur en niveaux de gris
    imageStore(outputImage, pixelCoord, color);
}
