#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

out vec4 color;

uniform sampler2D texsampler;

void main()
{
   vec3 c = texture( texsampler, uv ).rgb;

   if(length(c) < 0.1)
   {
       discard;
   }

   color = vec4(c, max(0.0, min(1.0, length(c) - 0.1)));
}