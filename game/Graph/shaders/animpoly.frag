#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 uv;
layout(location = 2) flat in int texId;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 inColor;

out vec4 color;

uniform sampler2D texsampler[64];

struct Light
{
    vec4 pos;
    vec4 lightColor;
    float fallstart;
    float fallend;
    float precalc;
};

layout(std140) uniform LightData
{
    Light lights[18];
};

uniform int numLights;

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
	
	vec3 lightContribution = inColor.rgb;

    //35 corresponds to Arx constant NPC_ITEMS__AMBIENT_VALUE_255
    lightContribution += vec3((35.0 / 255.0));

    int lightCount = 0;
	for(int i = 0; i < numLights; ++i)
    {
        Light l = lights[i];
        vec3 lightPos = l.pos.xyz;
		vec3 diff = lightPos - fragPos;
		float lightDist = length(diff);
        
        if(lightDist < l.fallend)
        {
			diff *= 1.0/lightDist;
			
			vec3 newnormal = vec3(normal.x, -normal.y, normal.z);
			float d = dot(diff, newnormal);

            if(d > 0.0)
            {
                float adj = 0.0;
				if(lightDist <= l.fallstart)
				{
					adj = d * l.precalc;
				}
				else
				{
					float falldiffmul = 1.0/(l.fallend - l.fallstart);
					float p = (l.fallend - lightDist) * falldiffmul;
					
					if(p <= 0.0)
						adj = 0.0;
					else
						adj = d * p * l.precalc;
				}
                
                //adj can be >=1.0 here which is what generates
                // the iridescence effect on some surfaces
                lightContribution += (l.lightColor.rgb * adj);
            }
        }
    }

    lightContribution.r = min(1.0, lightContribution.r);
    lightContribution.g = min(1.0, lightContribution.g);
    lightContribution.b = min(1.0, lightContribution.b);
    
    color = vec4(lightContribution * fragColor, inColor.a);
}