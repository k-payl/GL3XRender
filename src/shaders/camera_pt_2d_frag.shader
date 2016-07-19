#version 330

smooth in vec2 UV;

uniform sampler2D texture0;

out vec4 color;


void main()
{
	vec4 color_ = texture(texture0, UV);
	color = vec4(color_.r, 0, 0, 1);
}