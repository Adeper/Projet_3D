#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D noiseTexture; // La texture de bruit générée

void main()
{
    FragColor = texture(noiseTexture, TexCoords); // Afficher la texture de bruit
}
