#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

/* COMPARAISON DES DIFFERENTS BRUITS DE PERLIN*/

// Paramètres du bruit
uniform int noiseType;           // Type de bruit: 0 = Perlin simple, 1 = Simplex, 2 = Cohérent, 3 = Diamond-Square
uniform float noiseScale;         // Échelle du bruit
uniform float noiseGain;          // Gain du bruit
uniform int noiseOctaves;         // Nombre d'octaves
uniform float noisePersistence;   // Persistance du bruit
uniform float noisePower;         // Puissance du bruit
uniform int seed;                 // Graine pour le bruit

/* Source pour les algorithmes de bruit de perlin:
 * https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
*/

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
// Bruit cohérent simple (ex. interpolation de sinusoïdes)
float coherentNoise(vec2 uv) {
    uv *= noiseScale;
    float value = sin(uv.x) * cos(uv.y); // Exemple : sinusoïdes croisées
    return (value + 1.0) * 0.5;          // Normaliser dans [0, 1]
}

/* ----------- BRUIT DIAMOND-SQUARE ----------- */
float diamondSquare(vec2 uv) {
    vec2 p = floor(uv * noiseScale);
    vec2 offset = vec2(0.5, 0.5); // Simuler la division d'une grille
    float corners = (rando(p) + rando(p + vec2(1.0, 0.0)) +
                     rando(p + vec2(0.0, 1.0)) + rando(p + vec2(1.0, 1.0))) * 0.25;
    float edges = (rando(p + vec2(0.5, 0.0)) + rando(p + vec2(0.0, 0.5))) * 0.5;
    float center = rando(p + offset);
    return (corners + edges + center) * 0.3333; // Combinaison équilibrée
}

/* ----------- GENERATION DE BRUIT (toutes sources combinées) ----------- */
float generateNoise(vec2 uv) {
    float amplitude = noiseGain;
    float frequency = noiseScale;
    float total = 0.0;

    for (int i = 0; i < noiseOctaves; i++) {
        float value;
        if (noiseType == 0) {
            value = noise(uv * frequency); // Bruit Simple
        } else if (noiseType == 1) {
            value = snoise(uv * frequency); // Bruit Simplex
        } else if (noiseType == 2) {
            value = coherentNoise(uv * frequency); // Bruit cohérent
        } else if (noiseType == 3) {
            value = diamondSquare(uv * frequency); // Bruit Diamond-Square
        } else {
            value = 0.0; // Par défaut
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
    vec2 uv = TexCoords + vec2(float(seed) * 0.0001); // Ajout de graine pour varier les motifs
    float noiseValue = generateNoise(uv);
    FragColor = vec4(vec3(noiseValue), 1.0); // Valeur en niveaux de gris
}
