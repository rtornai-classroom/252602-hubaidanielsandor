#version 330 core

in vec2 fragPos;
out vec4 color;

uniform vec2 circleCenter;
uniform float radius;
uniform bool isLine;
uniform int colorSwap;

void main() {

    if (isLine) { //Line colouring
        color = vec4(0.0, 0.0, 1.0, 1.0);
        return;
    }

    vec2 pixelPos = vec2(
        fragPos.x * 300.0 + 300.0,
        fragPos.y * 300.0 + 300.0
    );

    float dist = distance(pixelPos, circleCenter);

    if (dist > radius) discard;

    float t = dist / radius;

    vec3 centerColor = vec3(1.0, 0.0, 0.0);
    vec3 edgeColor   = vec3(0.0, 1.0, 0.0);

    if (colorSwap == 1) { //Color swapping method
        vec3 temp = centerColor;
        centerColor = edgeColor;
        edgeColor = temp;
    }

    color = vec4(mix(centerColor, edgeColor, t), 1.0);
}