#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

// Paramètres du bruit
uniform int noiseType;           // Type de bruit: 0 = Perlin, 1 = Simplex, etc.
uniform float noiseScale;         // Échelle du bruit
uniform float noiseGain;          // Gain du bruit
uniform int noiseOctaves;         // Nombre d'octaves
uniform float noisePersistence;   // Persistance du bruit
uniform float noisePower;         // Puissance du bruit
uniform int seed;                 // Graine pour le bruit

// Fonction de hachage pour générer des valeurs pseudo-aléatoires
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float rando(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rando(ip),rando(ip+vec2(1.0,0.0)),u.x),
		mix(rando(ip+vec2(0.0,1.0)),rando(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

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

    // Normaliser (optionnel, selon la plage que tu veux afficher)
    total = clamp(total, 0.0, 1.0);
    
    return total;
}

void main()
{
    // Utiliser la graine pour varier le motif
    // vec2 uv = TexCoords + vec2(float(seed) * 0.0001);
    // float value = generateNoise(uv);

    /*DEBUG*/
    // 1. Affiche les coordonnées TexCoords : Validé
    // FragColor = vec4(TexCoords, 0.0, 1.0);

    // 2. Affiche uniquement le résultat brut de hash() : Validé
    // FragColor = vec4(vec3(hash(TexCoords)), 1.0);

    // 3. Affiche uniquement le résultat brut de rando() : Validé
    // FragColor = vec4(vec3(rando(TexCoords)), 1.0);

    // 4. Affiche uniquement le résultat brut de noise() : Validé ?
    // FragColor = vec4(vec3(noise(TexCoords)), 1.0);

    // 5. Affiche le résultat de generateNoise() : Validé
    vec2 uv = TexCoords; // Utilisation des coordonnées de texture
    float noiseValue = generateNoise(uv);
    FragColor = vec4(vec3(noiseValue), 1.0); // Valeur en niveaux de gris

}
