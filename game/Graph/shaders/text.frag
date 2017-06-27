#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

out vec4 color;

uniform sampler2D texsampler;
uniform vec3 fontColor;

void main()
{
    float texel = texture( texsampler, uv ).r;
    color = vec4(fontColor * texel, texel);
}