#version 330 core

layout(location = 0) in vec3 position;  
layout(location = 2) in vec3 normal;    

uniform mat4 model;      
uniform mat4 view;      
uniform mat4 projection;

out vec3 fragNormal;     
out vec3 fragPosition;   

void main() {
    vec4 worldPosition = model * vec4(position, 1.0);
    
    fragPosition = vec3(worldPosition);

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    fragNormal = normalize(normalMatrix * normal);

    gl_Position = vec4(worldPosition.xyz, 1.0);
}