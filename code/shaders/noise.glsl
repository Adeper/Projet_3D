#version 330 core

layout (local_size_x = 4, local_size_y = 4) in;
layout (rgba32f, binding = 0) uniform image2D noiseTexture;

uniform int noiseType;
uniform float noiseScale;
uniform float noiseGain;
uniform int noiseOctaves;
uniform float noisePersistence;
uniform float noisePower;
uniform int seed;

// Exemple de fonction de bruit
float test_noise(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    ivec2 texelCoords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = vec2(texelCoords) / vec2(imageSize(noiseTexture));

    float noiseValue = test_noise(uv * noiseScale);

    vec4 color = vec4(noiseValue, noiseValue, noiseValue, 1.0);
    imageStore(noiseTexture, texelCoords, color);
}

