#version 330 compatibility
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in int inTexId;

layout(location = 0) out vec2 uv;
layout(location = 1) out int texId;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    uv = inUV;
    texId = inTexId;

    gl_Position = proj * view * model * vec4(pos, 1.0);
}