#version 330 compatibility
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 uv;
layout(location = 2) flat in int texId;
layout(location = 3) in vec3 normal;

out vec3 color;

uniform sampler2D texsampler[24];

struct Light
{
    vec4 pos;
    vec4 lightColor;
    float fallstart;
    float _pad1[3];
    float fallend;
    float _pad2[3];
};

layout(std140) uniform LightData
{
    Light lights[18];
};

void main()
{
    vec3 fragColor = vec3(1.0, 0.0, 1.0);
    
    if(texId >= 0)
    {
        fragColor = texture( texsampler[texId], uv ).rgb;

        //Pure black is the alpha mask in textures for 3D models.
        if(fragColor == vec3(0.0, 0.0, 0.0))
        {
            discard;
        }
    }

    vec3 base = fragColor;

    int lightCount = 0;
    const int MAX_LIGHTS = 4;
    for(int i = 0; i < 18; ++i)
    {
        Light l = lights[i];
        float lightDist = distance(l.pos.xyz, fragPos);
        if(lightDist < l.fallend)
        {
            float d = dot(normalize(l.pos.xyz - fragPos), normal);
            d = abs(d); //TODO: fix.
            if(d >= 0.0)
            {
                float delta = ((lightDist - l.fallstart) / (l.fallend - l.fallstart));
                delta = max(0.0, delta);
                
                fragColor *= ((l.lightColor.rgb * (1.0 - delta)) * d);
                
                if(++lightCount >= MAX_LIGHTS)
                {
                    break;
                }
            }
        }
    }

    //If unlit, default to ambient.
    if(lightCount == 0)
        fragColor *= 0.0;

    color = fragColor + (base * 0.2);
}