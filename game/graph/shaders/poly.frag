#version 330 compatibility
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) flat in int texId;

out vec3 color;

uniform sampler2D texsampler[16];

void main()
{
    vec3 fragColor = vec3(1.0, 0.0, 1.0);
    
    if(texId >= 0)
    {
        fragColor = texture( texsampler[texId], uv ).rgb;
    }

    color = fragColor;
}