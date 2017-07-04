#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

out vec3 fragColor;

uniform sampler2D texsampler;
uniform vec3 color;

void main()
{
   fragColor = texture( texsampler, uv ).rgb;

   //Behaviour corresponds to RestoreFakeBlack in D3D7 Arx
   if(fragColor == vec3(0.0, 0.0, 0.0))
   {
       discard;
   }

   fragColor *= color;
}