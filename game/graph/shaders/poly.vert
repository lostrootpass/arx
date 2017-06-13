#version 330 compatibility
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in int inTexId;
layout(location = 3) in vec3 inNorm;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec2 uv;
layout(location = 2) out int texId;
layout(location = 3) out vec3 norm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    outPos = pos;
    uv = inUV;
    texId = inTexId;
    norm = inNorm;

    gl_Position = proj * view * model * vec4(pos, 1.0);
}