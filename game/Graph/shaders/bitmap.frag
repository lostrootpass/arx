#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

out vec3 color;

uniform sampler2D texsampler;

void main()
{
   color = texture( texsampler, uv ).rgb;

   //Behaviour corresponds to RestoreFakeBlack in D3D7 Arx
   if(color == vec3(0.0, 0.0, 0.0))
   {
       discard;
   }
}