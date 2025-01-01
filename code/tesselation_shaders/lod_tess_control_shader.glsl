#version 450 core

layout(vertices = 3) out;

uniform vec3 cameraPosition;
uniform float lodDistance;

in vec3 vs_position[];  // Position depuis le vertex shader
in vec2 vs_uv[]; // UV depuis le vertex shader

out vec3 tcs_position[];  // Position vers le tessellation evaluation shader
out vec2 tcs_uv[]; // UV vers le tessellation evaluation shader

void main() {

    float distanceToCamera = length(vs_position[gl_InvocationID] - cameraPosition);
    float factor = clamp(1.0 - (distanceToCamera / lodDistance), 0.0, 1.0);

    float tessellationLevel = mix(1.0, 9.0, factor);

    // Fixe le niveau de tessellation pour les niveaux intérieur et extérieur
    gl_TessLevelOuter[0] = tessellationLevel;
    gl_TessLevelOuter[1] = tessellationLevel;
    gl_TessLevelOuter[2] = tessellationLevel;

    gl_TessLevelInner[0] = tessellationLevel;

    // Transmettre les positions au TES
    tcs_position[gl_InvocationID] = vs_position[gl_InvocationID];
    tcs_uv[gl_InvocationID] = vs_uv[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
