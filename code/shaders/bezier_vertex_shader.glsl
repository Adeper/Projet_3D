#version 460 core

layout (location = 0) in vec3 position;

layout (std430, binding = 0) buffer ControlPoints {
    vec3 controlPoints[];
};

uniform float t;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 curvePos;

float factorial(int n) {
    if (n == 0) return 1.0;
    float result = 1.0;
    for (int i = 1; i <= n; ++i) {
        result *= float(i);
    }
    return result;
}

void main() {
    int n = int(controlPoints.length()) - 1;

    vec3 result = vec3(0.0);
    for (int i = 0; i <= n; ++i) {
        // Coefficients binomiaux de Bézier
        float binom = factorial(n) / (factorial(i) * factorial(n - i));
        float blend = binom * pow(1.0 - t, float(n - i)) * pow(t, float(i));
        result += blend * controlPoints[i];
    }

    curvePos = result;
    gl_Position = projection * view * model * vec4(curvePos, 1.0);
}