#version 330 core

layout (location = 0) in vec2 aPos;

uniform mat4 matProjection;
uniform mat4 matModelView;

void main() {
    gl_Position = matProjection * matModelView * vec4(aPos, 0.0, 1.0);
}