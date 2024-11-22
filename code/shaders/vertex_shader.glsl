#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV;
layout(location = 1) in vec3 aNormal;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
	FragPos = vec3(model * vec4(vertexPosition, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
    UV = vertexUV;
}
