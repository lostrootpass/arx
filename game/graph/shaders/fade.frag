#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

out vec4 color;

uniform vec4 colorUniform;

void main()
{
   color = colorUniform;
}