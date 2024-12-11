#version 330 core

layout(location = 0) in vec3 position;

out vec3 FragPos;

uniform mat4 model;      
uniform mat4 view;       
uniform mat4 projection; 

void main() {
    vec4 worldPosition = model * vec4(position, 1.0);
    FragPos = vec3(worldPosition);

    gl_Position = projection * view * worldPosition;
}
