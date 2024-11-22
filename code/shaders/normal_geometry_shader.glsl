#version 330 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 fragNormal[];
in vec3 fragPosition[];

uniform mat4 view;
uniform mat4 projection;
uniform float normalLength = 0.5;

void main() {
    vec3 start = fragPosition[0];

    vec3 end = start + fragNormal[0] * normalLength;

    gl_Position = projection * view * vec4(start, 1.0);
    EmitVertex();

    gl_Position = projection * view * vec4(end, 1.0);
    EmitVertex();

    EndPrimitive();
}
