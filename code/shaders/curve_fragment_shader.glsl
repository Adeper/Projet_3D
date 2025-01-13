#version 330 core

in vec2 TexCoord; 
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D curveTexture; 
uniform bool showRoad;         

void main() {
    if (showRoad) {
        FragColor = texture(curveTexture, TexCoord);
    } else {
        FragColor = vec4(color, 1.0);
    }
}
