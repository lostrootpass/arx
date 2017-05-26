#version 330 core

in vec2 uv;

out vec3 color;

uniform sampler2D texsampler;

void main()
{
   color = texture( texsampler, uv ).rgb;
}