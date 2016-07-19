#version 330

smooth in vec2 UV;

uniform sampler2D texture0;

out vec4 color;


void main()
{
	vec4 tex = texture(texture0, UV);
	color = vec4(1, 1, 1, tex.r);
}