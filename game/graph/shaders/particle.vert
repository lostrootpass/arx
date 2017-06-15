#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 uv;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    uv = inUV;
    gl_Position = proj * view * model * vec4(pos.xyz, 1.0);
}