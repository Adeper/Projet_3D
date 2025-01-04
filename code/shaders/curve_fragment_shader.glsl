#version 330 core

in vec2 UV; // Coordonnées UV
out vec4 FragColor;

uniform vec3 color;
uniform sampler2D curveTexture; // Texture de la courbe
uniform int useTexture;         // Indique si une texture est utilisée

void main() {
    if (useTexture == 1) {
        FragColor = texture(curveTexture, UV);
    } else {
        FragColor = vec4(color, 1.0);
    }
}
