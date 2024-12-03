#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform vec3 ambientColor;
uniform vec3 color_Mesh;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);

    // Composante diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Composante ambiante
    vec3 ambient = ambientColor * lightColor;

    // Couleur finale
    vec3 result = (ambient + diffuse) * color_Mesh;
    FragColor = vec4(result, 1.0);
}
