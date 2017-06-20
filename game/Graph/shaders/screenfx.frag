#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

out vec4 outColor;

uniform sampler2D texsampler;
uniform vec4 color;

void main()
{
    outColor = texture(texsampler, uv).rgba;
	outColor.a = outColor.r/2.0;
    outColor *= color;
}