#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) flat in int texId; 
layout(location = 2) in vec4 color;

out vec4 fragColor;

uniform sampler2D texsampler;

void main()
{
    if(texId > 0)
    {
        fragColor.rgb = texture( texsampler, uv ).rgb;
        fragColor.a = 1.0;
        fragColor *= color;
    }
    else
    {
        fragColor = color;
    }
}