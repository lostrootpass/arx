#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 fragPos;

out vec3 color;

struct Light
{
    vec4 pos;
    vec4 lightColor;
    float fallstart;
    float fallend;
};

layout(std140) uniform LightData
{
    Light light;
};
uniform sampler2D texsampler;

void main()
{
    vec3 base = texture( texsampler, uv ).rgb;
    float ra = length(light.pos.xy - fragPos.xy);

    if(ra > light.fallend)
    {
        color = base * 0.1;
    }
    else
    {
        vec3 adj = light.lightColor.rgb;

        ra = (ra < light.fallstart) ? 1.0 : (light.fallend - ra) / (light.fallend - light.fallstart);
        float t = max(0.1, light.lightColor.a * ra);
        
        adj *= t;

        color = base * adj;
    }
}