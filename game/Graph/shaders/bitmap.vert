#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inUV;

out vec2 uv;

uniform mat4 proj;

void main()
{
    uv = inUV;
    gl_Position = proj * vec4(pos, 1.0);
}