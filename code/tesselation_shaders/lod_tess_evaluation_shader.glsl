#version 450 core

layout(triangles, equal_spacing, ccw) in;

in vec3 tcs_position[]; // Données depuis le TCS
in vec2 tcs_uv[];

out vec3 tes_fragPos; 
out vec2 tes_uv;

uniform sampler2D heightMap;
uniform float heightScale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 p0 = tcs_position[0];
    vec3 p1 = tcs_position[1];
    vec3 p2 = tcs_position[2];

    vec2 uv0 = tcs_uv[0];
    vec2 uv1 = tcs_uv[1];
    vec2 uv2 = tcs_uv[2];

    vec3 position = p0 * gl_TessCoord.x + p1 * gl_TessCoord.y + p2 * gl_TessCoord.z;
    vec2 uv = uv0 * gl_TessCoord.x + uv1 * gl_TessCoord.y + uv2 * gl_TessCoord.z;

    float height = texture(heightMap, uv).r;
    position.y += height * heightScale;

    tes_fragPos = position; // Transmettre la position au FS
    tes_uv = uv;            // Transmettre les UV au FS

    gl_Position = projection * view * vec4(position, 1.0);
}
