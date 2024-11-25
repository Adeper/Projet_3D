#version 330 core

in vec2 UV;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform vec3 LightPosition_worldspace;
uniform vec3 LightColor_worldspace;
uniform vec3 color_Mesh;
uniform sampler2D heightmap;

void main() {
    float height = texture(heightmap, UV).r;
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(LightPosition_worldspace - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = diff * LightColor_worldspace;

    vec3 result = diffuse * color_Mesh;

    //color = vec4(result, 1.0);

    //color = vec4(color_Mesh, 1.0);

    color = vec4(mix(vec3(0.2,0.5,0.8), vec3(1.,1.,1.), height), 1.0);
}