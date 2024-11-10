#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

// Paramètres du bruit (à passer comme uniformes)
uniform int noiseType;           // Type de bruit: 0 = Perlin, 1 = Simplex, etc.
uniform float noiseScale;         // Échelle du bruit
uniform float noiseGain;          // Gain du bruit
uniform int noiseOctaves;         // Nombre d'octaves
uniform float noisePersistence;   // Persistance du bruit
uniform float noisePower;         // Puissance du bruit
uniform int seed;                 // Graine pour le bruit

// Fonction de génération de bruit (ex: bruit simple pour illustration)
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float generateNoise(vec2 uv) {
    uv *= noiseScale;

    float amplitude = noiseGain;
    float frequency = 1.0;
    float noiseValue = 0.0;

    for (int i = 0; i < noiseOctaves; ++i) {
        noiseValue += amplitude * noise(uv * frequency);
        frequency *= 2.0;
        amplitude *= noisePersistence;
    }

    return pow(noiseValue, noisePower);
}

void main()
{
    vec2 uv = TexCoords + vec2(seed); // Utiliser la graine pour varier
    float value = generateNoise(uv);
    FragColor = vec4(vec3(value), 1.0); // Résultat en niveaux de gris
}
