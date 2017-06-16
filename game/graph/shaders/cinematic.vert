#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 outPos;

uniform mat4 proj;

void main()
{
    uv = inUV;
    outPos = pos;
    
    gl_Position = proj * vec4(pos, 1.0);
}