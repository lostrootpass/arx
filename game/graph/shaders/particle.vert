#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in int inTexId;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec2 uv;
layout(location = 1) out int texId;
layout(location = 2) out vec4 color;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    uv = inUV;
    texId = inTexId;
    color = inColor;

    gl_Position = proj * view * model * vec4(pos.xyz, 1.0);
}