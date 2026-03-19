#version 330 core

in vec2 position;
out vec2 fragPos;

void main() {
    fragPos = position;
    gl_Position = vec4(position, 0.0, 1.0);
}