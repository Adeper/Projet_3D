#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 aNormal;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;
out float height;

uniform mat4 model;      
uniform mat4 view;      
uniform mat4 projection;
uniform sampler2D heightMap;
uniform float heightScale; 

void main() {
    float height_ = texture(heightMap, vertexUV).r;

    vec3 position = vertexPosition;
    position.y = height_ * heightScale;

    vec4 worldPosition = model * vec4(position, 1.0);
    FragPos = vec3(worldPosition);

    float dx = 1.0 / textureSize(heightMap, 0).x;
    float dy = 1.0 / textureSize(heightMap, 0).y;

    float heightL = texture(heightMap, vertexUV + vec2(-dx, 0.0)).r * heightScale;
    float heightR = texture(heightMap, vertexUV + vec2(dx, 0.0)).r * heightScale;
    float heightD = texture(heightMap, vertexUV + vec2(0.0, -dy)).r * heightScale;
    float heightU = texture(heightMap, vertexUV + vec2(0.0, dy)).r * heightScale;

    vec3 tangent = vec3(1.0, (heightR - heightL), 0.0);
    vec3 bitangent = vec3(0.0, (heightU - heightD), 1.0);

    vec3 localNormal = normalize(cross(tangent, bitangent));

    localNormal = localNormal*-1;

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalize(normalMatrix * localNormal);

    gl_Position = projection * view * worldPosition;

    UV = vertexUV;
    height = position.y;
}

