#version 330 core

in vec2 UV;
in vec3 Normal;
in vec3 FragPos;

//uniform vec3 LightPosition_worldspace;
//uniform vec3 LightColor_worldspace;
uniform vec3 ViewPosition;
uniform vec3 ambientColor;

uniform vec3 color_Mesh;

//uniform sampler2D texture1;

// Composantes spéculaires du matériau
//uniform float shininess;

out vec4 color;

void main() {
    vec3 LightPosition_worldspace = vec3(0.f, 1.f, 0.f);
    vec3 LightColor_worldspace = vec3(1.0f, 1.0f, 1.0f);


    // Propriétés du matériau (diffuse et spéculaire)
    // vec3 materialDiffuse = texture(texture1, TexCoords).rgb;
    vec3 materialDiffuse = color_Mesh;
    vec3 materialSpecular = color_Mesh;

    float shininess = 8.0f;

    // Composante ambiante
    vec3 ambient = ambientColor * materialDiffuse;

    // Composante diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPosition_worldspace - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * LightColor_worldspace * materialDiffuse;

    // Composante spéculaire
    vec3 viewDir = normalize(ViewPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * LightColor_worldspace;

    // Couleur finale
    vec3 result = ambient + diffuse + specular;
    //color = vec4(result, 1.0);

    color = vec4(specular, 1.0);

    //color = vec4(norm, 1.0);
}